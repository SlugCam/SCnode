/* 
 * File:   main.c
 * Author: Caio
 *
 * Created on May 22, 2013, 1:52 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <string>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core_c.h>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

extern "C" {
	#include "bcm_host.h"
	#include "interface/vcos/vcos.h"

	#include "interface/mmal/mmal.h"
	#include "interface/mmal/mmal_logging.h"
	#include "interface/mmal/mmal_buffer.h"
	#include "interface/mmal/util/mmal_util.h"
	#include "interface/mmal/util/mmal_util_params.h"
	#include "interface/mmal/util/mmal_default_components.h"
	#include "interface/mmal/util/mmal_connection.h"
	#include "RaspiCamControl.h"

	#include "vgfont.h"
}

using namespace cv;
using namespace std;

#define MMAL_CAMERA_PREVIEW_PORT 0
#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_CAPTURE_PORT 2

#define OPENCV_CONFIG_FRAMES 100

int learnedFrames = 0;

typedef struct {
    int video_width;
    int video_height;
    int preview_width;
    int preview_height;
    int opencv_width;
    int opencv_height;
    float video_fps;
    MMAL_POOL_T *camera_video_port_pool;
    Mat image;
    Mat image2;
    VCOS_SEMAPHORE_T complete_semaphore;
} PORT_USERDATA;

static void video_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
    static int frame_post_count = 0;
    MMAL_BUFFER_HEADER_T *new_buffer;
    PORT_USERDATA * userdata = (PORT_USERDATA *) port->userdata;
    MMAL_POOL_T *pool = userdata->camera_video_port_pool;

    mmal_buffer_header_mem_lock(buffer);
    memcpy(userdata->image.data, buffer->data, userdata->video_width * userdata->video_height);
    mmal_buffer_header_mem_unlock(buffer);

    if (vcos_semaphore_trywait(&(userdata->complete_semaphore)) != VCOS_SUCCESS) {
        vcos_semaphore_post(&(userdata->complete_semaphore));
        frame_post_count++;
    }

    mmal_buffer_header_release(buffer);
    // and send one back to the port (if still open)
    if (port->is_enabled) {
        MMAL_STATUS_T status;

        new_buffer = mmal_queue_get(pool->queue);

        if (new_buffer)
            status = mmal_port_send_buffer(port, new_buffer);

        if (!new_buffer || status != MMAL_SUCCESS)
            printf("Unable to return a buffer to the video port\n");
    }
}

int set_camera_parameters(MMAL_COMPONENT_T *camera, const RASPICAM_CAMERA_PARAMETERS params)
{
   int result;

   result  = raspicamcontrol_set_saturation(camera, params.saturation);
   result += raspicamcontrol_set_sharpness(camera, params.sharpness);
   result += raspicamcontrol_set_contrast(camera, params.contrast);
   result += raspicamcontrol_set_brightness(camera, params.brightness);
   //result += raspicamcontrol_set_ISO(camera, params.ISO); TODO Not working for some reason
   result += raspicamcontrol_set_video_stabilisation(camera, params.videoStabilisation);
   result += raspicamcontrol_set_exposure_compensation(camera, params.exposureCompensation);
   result += raspicamcontrol_set_exposure_mode(camera, params.exposureMode);
   result += raspicamcontrol_set_metering_mode(camera, params.exposureMeterMode);
   result += raspicamcontrol_set_awb_mode(camera, params.awbMode);
   result += raspicamcontrol_set_imageFX(camera, params.imageEffect);
   result += raspicamcontrol_set_colourFX(camera, &params.colourEffects);
   //result += raspicamcontrol_set_thumbnail_parameters(camera, &params.thumbnailConfig);  TODO Not working for some reason
   result += raspicamcontrol_set_rotation(camera, params.rotation);
   result += raspicamcontrol_set_flips(camera, params.hflip, params.vflip);

   return result;
}

int main() {
    MMAL_COMPONENT_T *camera = 0;
    MMAL_COMPONENT_T *preview = 0;
    MMAL_ES_FORMAT_T *format;
    MMAL_STATUS_T status;
    MMAL_PORT_T *camera_preview_port = NULL, *camera_video_port = NULL, *camera_still_port = NULL;
    MMAL_PORT_T *preview_input_port = NULL;
    MMAL_POOL_T *camera_video_port_pool;
    MMAL_CONNECTION_T *camera_preview_connection = 0;
    PORT_USERDATA userdata;

    int display_width, display_height;

    printf("Running...\n");

    bcm_host_init();

	//size of the preview image
    userdata.preview_width = 1280 / 1;
    userdata.preview_height = 720 / 1;

	//size of the image captured from the camera
    userdata.video_width = 1280 / 1;
    userdata.video_height = 720 / 1;

	//size of the image processed by the OpenCV
	int scale = 3;
    userdata.opencv_width = 1280 / scale;
    userdata.opencv_height = 720 / scale;

    display_width=1920;
	display_height=1080;

    float r_w, r_h;
    r_w = (float) display_width / (float) userdata.opencv_width;
    r_h = (float) display_height / (float) userdata.opencv_height;

    printf("Display resolution = (%d, %d)\n", display_width, display_height);

    /* setup opencv */
    userdata.image = Mat(Size(userdata.video_width, userdata.video_height), CV_8UC1);
    userdata.image2 = Mat(Size(userdata.opencv_width, userdata.opencv_height), CV_8UC1);

    status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera);
    if (status != MMAL_SUCCESS) {
        printf("Error: create camera %x\n", status);
        return -1;
    }

    camera_preview_port = camera->output[MMAL_CAMERA_PREVIEW_PORT];
    camera_video_port = camera->output[MMAL_CAMERA_VIDEO_PORT];
    camera_still_port = camera->output[MMAL_CAMERA_CAPTURE_PORT];

    MMAL_PARAMETER_CAMERA_CONFIG_T cam_config = {
        { MMAL_PARAMETER_CAMERA_CONFIG, sizeof (cam_config)},
        .max_stills_w = 1280,
        .max_stills_h = 720,
        .stills_yuv422 = 0,
        .one_shot_stills = 0,
        .max_preview_video_w = 1280,
        .max_preview_video_h = 720,
        .num_preview_video_frames = 2,
        .stills_capture_circular_buffer_height = 0,
        .fast_preview_resume = 1,
        .use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RESET_STC
    };

    mmal_port_parameter_set(camera->control, &cam_config.hdr);

    format = camera_video_port->format;

    format->encoding = MMAL_ENCODING_I420;
    format->encoding_variant = MMAL_ENCODING_I420;

    format->es->video.width = userdata.video_width;
    format->es->video.height = userdata.video_width;
    format->es->video.crop.x = 0;
    format->es->video.crop.y = 0;
    format->es->video.crop.width = userdata.video_width;
    format->es->video.crop.height = userdata.video_height;
    format->es->video.frame_rate.num = 30;
    format->es->video.frame_rate.den = 1;

    camera_video_port->buffer_size = userdata.preview_width * userdata.preview_height * 12 / 8;
    camera_video_port->buffer_num = 1;
    printf("  Camera video buffer_size = %d\n", camera_video_port->buffer_size);

    status = mmal_port_format_commit(camera_video_port);

    if (status != MMAL_SUCCESS) {
        printf("Error: unable to commit camera video port format (%u)\n", status);
        return -1;
    }

    format = camera_preview_port->format;

    format->encoding = MMAL_ENCODING_OPAQUE;
    format->encoding_variant = MMAL_ENCODING_I420;

    format->es->video.width = userdata.preview_width;
    format->es->video.height = userdata.preview_height;
    format->es->video.crop.x = 0;
    format->es->video.crop.y = 0;
    format->es->video.crop.width = userdata.preview_width;
    format->es->video.crop.height = userdata.preview_height;

    status = mmal_port_format_commit(camera_preview_port);

    if (status != MMAL_SUCCESS) {
        printf("Error: camera viewfinder format couldn't be set\n");
        return -1;
    }

    // crate pool form camera video port
    camera_video_port_pool = (MMAL_POOL_T *) mmal_port_pool_create(camera_video_port, camera_video_port->buffer_num, camera_video_port->buffer_size);
    userdata.camera_video_port_pool = camera_video_port_pool;
    camera_video_port->userdata = (struct MMAL_PORT_USERDATA_T *) &userdata;

    status = mmal_port_enable(camera_video_port, video_buffer_callback);
    if (status != MMAL_SUCCESS) {
        printf("Error: unable to enable camera video port (%u)\n", status);
        return -1;
    }



    status = mmal_component_enable(camera);

    status = mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_RENDERER, &preview);
    if (status != MMAL_SUCCESS) {
        printf("Error: unable to create preview (%u)\n", status);
        return -1;
    }

	//Setting parameters for the camera
	RASPICAM_CAMERA_PARAMETERS params;

	params.sharpness = 0;
	params.contrast = 0;
	params.brightness = 50;
	params.saturation = 0;
	params.ISO = 400;
	params.videoStabilisation = 0;
	params.exposureCompensation = 0;
	params.exposureMode = MMAL_PARAM_EXPOSUREMODE_OFF;
	params.exposureMeterMode = MMAL_PARAM_EXPOSUREMETERINGMODE_AVERAGE;
	params.awbMode = MMAL_PARAM_AWBMODE_OFF;
	params.imageEffect = MMAL_PARAM_IMAGEFX_NONE;
	params.colourEffects.enable = 0;
	params.colourEffects.u = 128;
	params.colourEffects.v = 128;
	params.rotation = 180;
	params.hflip = params.vflip = 0;

	set_camera_parameters(camera, params);

    preview_input_port = preview->input[0];

    MMAL_DISPLAYREGION_T param;
    param.hdr.id = MMAL_PARAMETER_DISPLAYREGION;
    param.hdr.size = sizeof (MMAL_DISPLAYREGION_T);
    param.set = MMAL_DISPLAY_SET_LAYER;
    param.layer = 0;
    param.set |= MMAL_DISPLAY_SET_FULLSCREEN;
    param.fullscreen = 1;
    status = mmal_port_parameter_set(preview_input_port, &param.hdr);
    if (status != MMAL_SUCCESS && status != MMAL_ENOSYS) {
        printf("Error: unable to set preview port parameters (%u)\n", status);
        return -1;
    }

	//show the preview
    /*status = mmal_connection_create(&camera_preview_connection, camera_preview_port, preview_input_port, MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT);
    if (status != MMAL_SUCCESS) {
        printf("Error: unable to create connection (%u)\n", status);
        return -1;
    }

    status = mmal_connection_enable(camera_preview_connection);
    if (status != MMAL_SUCCESS) {
        printf("Error: unable to enable connection (%u)\n", status);
        return -1;
    }*/

    // Send all the buffers to the camera video port
    int num = mmal_queue_length(camera_video_port_pool->queue);
    int q;

    for (q = 0; q < num; q++) {
        MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(camera_video_port_pool->queue);

        if (!buffer) {
            printf("Unable to get a required buffer %d from pool queue\n", q);
        }

        if (mmal_port_send_buffer(camera_video_port, buffer) != MMAL_SUCCESS) {
            printf("Unable to send a buffer to encoder output port (%d)\n", q);
        }
    }

    if (mmal_port_parameter_set_boolean(camera_video_port, MMAL_PARAMETER_CAPTURE, 1) != MMAL_SUCCESS) {
        printf("%s: Failed to start capture\n", __func__);
    }

    vcos_semaphore_create(&userdata.complete_semaphore, "mmal_opencv_demo-sem", 0);
    int opencv_frames = 0;

	//namedWindow("Display window", CV_WINDOW_AUTOSIZE);
	//namedWindow("Display window2", CV_WINDOW_AUTOSIZE);
	const int varThreshold = 9;
	const bool bShadowDetection = false;
	const int history = 0;
	BackgroundSubtractorMOG2 bg(history,varThreshold,bShadowDetection);

	opencv_frames = 0;

	/////////////////////////////////////////
	///////////Config Params/////////////////
	/////////////////////////////////////////

	int dilate_n = 5;
	int erode_n = 1;
	opencv_frames=OPENCV_CONFIG_FRAMES;


    while (1) {
        if (vcos_semaphore_wait(&(userdata.complete_semaphore)) == VCOS_SUCCESS) {
            opencv_frames++;

			//Read configuration file
			/*if(opencv_frames >= OPENCV_CONFIG_FRAMES)
			{
				string line="";
				ifstream file;

				file.open("config");

				while(!file.eof())
				{
					string parameter;
					string value;

					getline(file, line);
					if(line.length()>=3)
					{
						parameter = line.substr(0,line.find('='));
						value = line.substr(line.find('=')+1);
						cout<<"parameter:"<<parameter<<endl;
						cout<<"value:"<<value<<endl;
						if(parameter == "dilate_n")
						{
							int aux;
							istringstream ( value ) >> aux;
							dilate_n=aux;
						}
						else if(parameter == "erode_n")
						{
							int aux;
							istringstream ( value ) >> aux;
							erode_n=aux;
						}
					}

				}

				opencv_frames=0;
			}*/

			//Resizes the userdata.image and saves on userdata.image2 with the size of userdata.image2
            resize(userdata.image, userdata.image2, userdata.image2.size(), 0, 0, CV_INTER_LINEAR);
			
			//subtract and find the contours
			Mat fore;
			bg.operator() (userdata.image2,fore);

			erode(fore, fore, Mat(), Point(-1,-1), erode_n);
			dilate(fore, fore, Mat(), Point(-1,-1), dilate_n);

			//imshow( "Display window", fore);

			vector<vector<cv::Point> > contours;

			findContours( fore, // binary input image 
                            contours, // vector of vectors of points
                            CV_RETR_EXTERNAL, // retrieve only external contours
                            CV_CHAIN_APPROX_SIMPLE); // aproximate contours
			
			//verify if the contour is a human
			for(int i=0;i < contours.size();i++)
			{
				vector<cv::Point> aux;
				approxPolyDP(contours[i], aux, 2, true);
				double x = arcLength(aux, true);
				if(x>100)
				{
					Rect body_i = boundingRect(contours[i]);

					if(body_i.height>2*body_i.width)
					{

						printf("  Human %d [%d, %d, %d, %d] [%d, %d, %d, %d]\n", i, body_i.x, body_i.y, body_i.width, body_i.height, (int) ((float) body_i.x * r_w), (int) (body_i.y * r_h), (int) (body_i.width * r_w), (int) (body_i.height * r_h));

						rectangle(userdata.image2, 
									body_i, 
									Scalar(0,255,255), 
									3 
									);
					}
					else
					{
						printf("  Thing %d [%d, %d, %d, %d] [%d, %d, %d, %d]\n", i, body_i.x, body_i.y, body_i.width, body_i.height, (int) ((float) body_i.x * r_w), (int) (body_i.y * r_h), (int) (body_i.width * r_w), (int) (body_i.height * r_h));

						rectangle(userdata.image2, 
									body_i, 
									Scalar(0,255,255), 
									3 
									);
					}
				}
			}
			if(opencv_frames%100 == 0)
			{
				ostringstream ss;
				ss << opencv_frames/100;
				imwrite("/home/test/test"+ss.str()+".jpg", userdata.image); 
				
			}
			//imshow( "Display window2", userdata.image2);
			
			//char key = (char) waitKey(1);
        }
    }
    return 0;
}

