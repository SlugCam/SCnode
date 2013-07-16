/* 
 * File:   main.c
 * Author: Caio
 *
 * Created on May 22, 2013, 1:52 PM
 */

#include <stdio.h>
#include <stdlib.h>

#include <opencv2/core/core_c.h>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "bcm_host.h"
#include "interface/vcos/vcos.h"

#include "interface/mmal/mmal.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"

#include "vgfont.h"

using namespace cv;

#define MMAL_CAMERA_PREVIEW_PORT 0
#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_CAPTURE_PORT 2

typedef struct {
    int video_width;
    int video_height;
    int preview_width;
    int preview_height;
    int opencv_width;
    int opencv_height;
    float video_fps;
    MMAL_POOL_T *camera_video_port_pool;
    CvHaarClassifierCascade *cascade;
    CvMemStorage* storage;
    IplImage* image;
    IplImage* image2;
    VCOS_SEMAPHORE_T complete_semaphore;
} PORT_USERDATA;

static void video_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer) {
    static int frame_count = 0;
    static int frame_post_count = 0;
    static struct timespec t1;
    struct timespec t2;
    MMAL_BUFFER_HEADER_T *new_buffer;
    PORT_USERDATA * userdata = (PORT_USERDATA *) port->userdata;
    MMAL_POOL_T *pool = userdata->camera_video_port_pool;

    if (frame_count == 0) {
        clock_gettime(CLOCK_MONOTONIC, &t1);
    }
    frame_count++;





    //img = cvLoadImage("test.jpg",CV_LOAD_IMAGE_COLOR);
    mmal_buffer_header_mem_lock(buffer);
    memcpy(userdata->image->imageData, buffer->data, userdata->video_width * userdata->video_height);
    mmal_buffer_header_mem_unlock(buffer);
    //printf("img = %d w=%d, h=%d\n", img, img->width, img->height);

    if (vcos_semaphore_trywait(&(userdata->complete_semaphore)) != VCOS_SUCCESS) {
        vcos_semaphore_post(&(userdata->complete_semaphore));
        frame_post_count++;
    }

    if (frame_count % 10 == 0) {
        // print framerate every n frame
        clock_gettime(CLOCK_MONOTONIC, &t2);
        float d = (t2.tv_sec + t2.tv_nsec / 1000000000.0) - (t1.tv_sec + t1.tv_nsec / 1000000000.0);
        float fps = 0.0;

        if (d > 0) {
            fps = frame_count / d;
        } else {
            fps = frame_count;
        }
        userdata->video_fps = fps;
        printf("  Frame = %d, Frame Post %d, Framerate = %.0f fps \n", frame_count, frame_post_count, fps);
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

/** @function detectAndDisplay */
void detectAndDisplay( Mat frame )
{
  std::vector<Rect> faces;
  Mat frame_gray;

  cvtColor( frame, frame_gray, CV_BGR2GRAY );
  equalizeHist( frame_gray, frame_gray );

  //-- Detect faces
  face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30) );

  for( int i = 0; i < faces.size(); i++ )
  {
    Point center( faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5 );
    ellipse( frame, center, Size( faces[i].width*0.5, faces[i].height*0.5), 0, 0, 360, Scalar( 255, 0, 255 ), 4, 8, 0 );

    Mat faceROI = frame_gray( faces[i] );
    std::vector<Rect> eyes;

    //-- In each face, detect eyes
    eyes_cascade.detectMultiScale( faceROI, eyes, 1.1, 2, 0 |CV_HAAR_SCALE_IMAGE, Size(30, 30) );

    for( int j = 0; j < eyes.size(); j++ )
     {
       Point center( faces[i].x + eyes[j].x + eyes[j].width*0.5, faces[i].y + eyes[j].y + eyes[j].height*0.5 );
       int radius = cvRound( (eyes[j].width + eyes[j].height)*0.25 );
       circle( frame, center, radius, Scalar( 255, 0, 0 ), 4, 8, 0 );
     }
  }
  //-- Show what you got
  imshow( window_name, frame );
 }

int main(int argc, char** argv) {
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
    userdata.opencv_width = 1280 / 1;
    userdata.opencv_height = 720 / 1;


    display_width = 1920;
    display_height = 1080;

    float r_w, r_h;
    r_w = (float) display_width / (float) userdata.opencv_width;
    r_h = (float) display_height / (float) userdata.opencv_height;

    printf("Display resolution = (%d, %d)\n", display_width, display_height);

    /* setup opencv */
    userdata.cascade = (CvHaarClassifierCascade*) cvLoad("/usr/share/opencv/haarcascades/haarcascade_frontalface_alt.xml", NULL, NULL, NULL); //Load the face detection object
    userdata.storage = cvCreateMemStorage(0);
    userdata.image = cvCreateImage(cvSize(userdata.video_width, userdata.video_height), IPL_DEPTH_8U, 1);
    userdata.image2 = cvCreateImage(cvSize(userdata.opencv_width, userdata.opencv_height), IPL_DEPTH_8U, 1);
    if (!userdata.cascade) {
        printf("Error: unable to load harrcascade\n");
    }
    //printf("Load cascade at %d\n", userdata.cascade);

    status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera);
    if (status != MMAL_SUCCESS) {
        printf("Error: create camera %x\n", status);
        return -1;
    }

    camera_preview_port = camera->output[MMAL_CAMERA_PREVIEW_PORT];
    camera_video_port = camera->output[MMAL_CAMERA_VIDEO_PORT];
    camera_still_port = camera->output[MMAL_CAMERA_CAPTURE_PORT];

    {
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
    }

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
	MMAL_PARAMETER_COLOURFX_T colfx = {{MMAL_PARAMETER_COLOUR_EFFECT,sizeof(colfx)}, 0, 0, 0};

	if (!camera)
		return 1;

	colfx.enable = 1;
	colfx.u = 128;
	colfx.v = 128;
	mmal_port_parameter_set(camera->control, &colfx.hdr); //Setting black and white

	int rotation = 180; //180 degrees of rotation
	int my_rotation = ((rotation % 360 ) / 90) * 90;

	mmal_port_parameter_set_int32(camera->output[0], MMAL_PARAMETER_ROTATION, my_rotation);	//Setting rotation
	mmal_port_parameter_set_int32(camera->output[1], MMAL_PARAMETER_ROTATION, my_rotation);	//Setting rotation
	mmal_port_parameter_set_int32(camera->output[2], MMAL_PARAMETER_ROTATION, my_rotation); //Setting rotation

    preview_input_port = preview->input[0];

    {
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
    }

    status = mmal_connection_create(&camera_preview_connection, camera_preview_port, preview_input_port, MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT);
    if (status != MMAL_SUCCESS) {
        printf("Error: unable to create connection (%u)\n", status);
        return -1;
    }

    status = mmal_connection_enable(camera_preview_connection);
    if (status != MMAL_SUCCESS) {
        printf("Error: unable to enable connection (%u)\n", status);
        return -1;
    }

    if (1) {
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


    }

    if (mmal_port_parameter_set_boolean(camera_video_port, MMAL_PARAMETER_CAPTURE, 1) != MMAL_SUCCESS) {
        printf("%s: Failed to start capture\n", __func__);
    }

    vcos_semaphore_create(&userdata.complete_semaphore, "mmal_opencv_demo-sem", 0);
    int opencv_frames = 0;
    struct timespec t1;
    struct timespec t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);

    GRAPHICS_RESOURCE_HANDLE img_overlay;
    GRAPHICS_RESOURCE_HANDLE img_overlay2;

    gx_graphics_init("/opt/vc/src/hello_pi/hello_font");

    gx_create_window(0, userdata.opencv_width, userdata.opencv_height, GRAPHICS_RESOURCE_RGBA32, &img_overlay);
    gx_create_window(0, 500, 200, GRAPHICS_RESOURCE_RGBA32, &img_overlay2);
    graphics_resource_fill(img_overlay, 0, 0, GRAPHICS_RESOURCE_WIDTH, GRAPHICS_RESOURCE_HEIGHT, GRAPHICS_RGBA32(0xff, 0, 0, 0x55));
    graphics_resource_fill(img_overlay2, 0, 0, GRAPHICS_RESOURCE_WIDTH, GRAPHICS_RESOURCE_HEIGHT, GRAPHICS_RGBA32(0xff, 0, 0, 0x55));

    graphics_display_resource(img_overlay, 0, 1, 0, 0, display_width, display_height, VC_DISPMAN_ROT0, 1);
    char text[256];

    while (1) {
        if (vcos_semaphore_wait(&(userdata.complete_semaphore)) == VCOS_SUCCESS) {
            opencv_frames++;
            float fps = 0.0;
            if (1) {
                clock_gettime(CLOCK_MONOTONIC, &t2);
                float d = (t2.tv_sec + t2.tv_nsec / 1000000000.0) - (t1.tv_sec + t1.tv_nsec / 1000000000.0);
                if (d > 0) {
                    fps = opencv_frames / d;
                } else {
                    fps = opencv_frames;
                }

                printf("  OpenCV Frame = %d, Framerate = %.2f fps \n", opencv_frames, fps);
            }

            graphics_resource_fill(img_overlay, 0, 0, GRAPHICS_RESOURCE_WIDTH, GRAPHICS_RESOURCE_HEIGHT, GRAPHICS_RGBA32(0, 0, 0, 0x00));
            graphics_resource_fill(img_overlay2, 0, 0, GRAPHICS_RESOURCE_WIDTH, GRAPHICS_RESOURCE_HEIGHT, GRAPHICS_RGBA32(0, 0, 0, 0x00));


            if (1) {
				//Convert IplImage to Mat
				Mat imgmat(userdata.image,false);
				//Detect
				detectAndDisplay( userdata.image );
				//// Resizes the userdata.image and saves on userdata.image2 with the size of userdata.image2
    //            cvResize(userdata.image, userdata.image2, CV_INTER_LINEAR);
				//// Detects the objects in the userdata.image2
    //            CvSeq* objects = cvHaarDetectObjects(userdata.image2, userdata.cascade, userdata.storage, 1.4, 3, 0, cvSize(100, 100), cvSize(150, 150));
    //            CvRect* r;
    //            // Loop through objects and draw boxes
    //            if (objects != 0) {
    //                if (objects->total > 0) {
    //                    int ii;
    //                    for (ii = 0; ii < objects->total; ii++) {
    //                        r = (CvRect*) cvGetSeqElem(objects, ii);

    //                        printf("  Face %d [%d, %d, %d, %d] [%d, %d, %d, %d]\n", ii, r->x, r->y, r->width, r->height, (int) ((float) r->x * r_w), (int) (r->y * r_h), (int) (r->width * r_w), (int) (r->height * r_h));


    //                        graphics_resource_fill(img_overlay, r->x, r->y, r->width, r->height, GRAPHICS_RGBA32(0xff, 0, 0, 0x88));
    //                        graphics_resource_fill(img_overlay, r->x + 4, r->y + 4, r->width - 8, r->height - 8, GRAPHICS_RGBA32(0, 0, 0, 0x00));


    //                    }
    //                } else {
    //                    //printf("No Face detected\n");
    //                }
    //            } else {
    //                printf("!! Face detectiona failed !!\n");
    //            }

            }

            sprintf(text, "Video = %.2f FPS, OpenCV = %.2f FPS", userdata.video_fps, fps);
            graphics_resource_render_text_ext(img_overlay2, 0, 0,
                    GRAPHICS_RESOURCE_WIDTH,
                    GRAPHICS_RESOURCE_HEIGHT,
                    GRAPHICS_RGBA32(0x00, 0xff, 0x00, 0xff), // foreground color
                    GRAPHICS_RGBA32(0, 0, 0, 0x00), // background color
                    text, strlen(text), 25);

            graphics_display_resource(img_overlay, 0, 1, 0, 0, display_width, display_height, VC_DISPMAN_ROT0, 1);
            graphics_display_resource(img_overlay2, 0, 2, 0, display_width / 16, GRAPHICS_RESOURCE_WIDTH, GRAPHICS_RESOURCE_HEIGHT, VC_DISPMAN_ROT0, 1);

        }
    }

    return 0;
}

