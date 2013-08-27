#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <string>
#include <time.h>

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
	#include "RaspiPreview.h"
	#include "RaspiCLI.h"

	#include "vgfont.h"
}

using namespace cv;
using namespace std;
/// Camera number to use - we only have one camera, indexed from 0.
#define CAMERA_NUMBER 0

// Standard port setting for the camera component
#define MMAL_CAMERA_PREVIEW_PORT 0
#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_CAPTURE_PORT 2

// Video format information
#define VIDEO_FRAME_RATE_NUM 30
#define VIDEO_FRAME_RATE_DEN 1

/// Video render needs at least 2 buffers.
#define VIDEO_OUTPUT_BUFFERS_NUM 3

// Max bitrate we allow for recording
const int MAX_BITRATE = 30000000; // 30Mbits/s

/// Interval at which we check for an failure abort during capture
const int ABORT_INTERVAL = 100; // ms

/// Open cv numbers of frames processed
#define OPENCV_CONFIG_FRAMES 100


int mmal_status_to_int(MMAL_STATUS_T status);

/** Structure containing all state information for the current run
 */
typedef struct
{
   int timeout;                        /// Time taken before frame is grabbed and app then shuts down. Units are milliseconds
   int width;                          /// Requested width of image
   int height;                         /// requested height of image
   int bitrate;                        /// Requested bitrate
   int framerate;                      /// Requested frame rate (fps)
   int intraperiod;                    /// Intra-refresh period (key frame rate)
   char *filename;                     /// filename of output file
   int verbose;                        /// !0 if want detailed run information
   int demoMode;                       /// Run app in demo mode
   int demoInterval;                   /// Interval between camera settings changes
   int opencv_width;                   /// Size of the opencv image
   int opencv_height;				   /// Size of the opencv image
   int maxLength;					   /// Number of frames on each video
   int consecutiveHumans;			   /// Minimum number of consecutive humans detection to continue recording
   int consecutiveNoHumans;			   /// Minimum number of consecutive frames without humans to stop recording
   RASPIPREVIEW_PARAMETERS preview_parameters;   /// Preview setup parameters
   RASPICAM_CAMERA_PARAMETERS camera_parameters; /// Camera setup parameters

   MMAL_COMPONENT_T *camera_component;    /// Pointer to the camera component
   MMAL_CONNECTION_T *preview_connection; /// Pointer to the connection from camera to preview

   MMAL_POOL_T *video_pool; /// Pointer to the pool of buffers used by video port
} RASPIVID_STATE;

/** Struct used to pass information in video port userdata to callback
 */
typedef struct
{
   string fileName;							/// File name
   VideoWriter fileHandle;					/// File handle to write buffer data to.
   VideoWriter fileHandle2;					/// File handle to write buffer data to.
   RASPIVID_STATE *pstate;					/// pointer to our state in case required in callback
   int abort;								/// Set to 1 in callback if an error occurs to attempt to abort the capture
   Mat image;								/// Main image captured
   Mat image2;								/// Image processed in opencCV
   int humanDetected;						/// Number of consecutive frames with human detected
   int noHumanDetected;						/// Number of consecutive frames without human
   int framesRecorded;						/// Number of frames recorded
   bool uploading;							/// Control if any file is being uploaded.
   VCOS_SEMAPHORE_T complete_semaphore;		
} PORT_USERDATA;

/// Command ID's and Structure defining our command line options
#define CommandHelp         0
#define CommandWidth        1
#define CommandHeight       2
#define CommandBitrate      3
#define CommandOutput       4
#define CommandVerbose      5
#define CommandTimeout      6
#define CommandDemoMode     7
#define CommandFramerate    8
#define CommandPreviewEnc   9
#define CommandIntraPeriod  10

/**
 * Assign a default set of parameters to the state passed in
 *
 * @param state Pointer to state structure to assign defaults to
 */
static void default_status(RASPIVID_STATE *state)
{
   if (!state)
   {
      vcos_assert(0);
      return;
   }

   // Default everything to zero
   memset(state, 0, sizeof(RASPIVID_STATE));

   // Now set anything non-zero
   state->timeout = 5000;     // 5s delay before take image
   state->width = 1280;       // Default to 1080p
   state->height = 720;
   state->bitrate = 17000000; // This is a decent default bitrate for 1080p
   state->framerate = VIDEO_FRAME_RATE_NUM;
   state->intraperiod = 0;    // Not set
   state->verbose = 0;    
   state->demoMode = 0;
   state->demoInterval = 250; // ms

   // Setup preview window defaults
   raspipreview_set_defaults(&state->preview_parameters);

   // Set up the camera_parameters to default
   raspicamcontrol_set_defaults(&state->camera_parameters);
}

/**
 *  buffer header callback function for camera control
 *
 *  Callback will dump buffer data to the specific file
 *
 * @param port Pointer to port from which callback originated
 * @param buffer mmal buffer header pointer
 */
static void camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
	MMAL_BUFFER_HEADER_T *new_buffer;
	PORT_USERDATA * userdata = (PORT_USERDATA *) port->userdata;
	RASPIVID_STATE *pstate = userdata->pstate;

	mmal_buffer_header_mem_lock(buffer);
	memcpy(userdata->image.data, buffer->data, userdata->pstate->width * userdata->pstate->height);

	userdata->fileHandle << userdata->image;
	userdata->framesRecorded++;
	// Check if the video didn't record humans in the first 3 frames
	if(userdata->framesRecorded > 4 && userdata->framesRecorded < pstate->maxLength && userdata->humanDetected < pstate->consecutiveHumans)
	{
		//remove(userdata->fileName.c_str());
		//system("poweroff");
	}
	// Check if the video is less then the maximun length and didn't have human in the pre determinated time
	else if(userdata->framesRecorded < pstate->maxLength && userdata->noHumanDetected >= pstate->consecutiveNoHumans)
	{
		//stop recording and wait until finished upload to power off
		//while(uploading){};
		//system("poweroff");
		//cout<<"No humans for a while\n";
	}
	// Check if the video has reached the maximum length and open a new one
	else if(userdata->framesRecorded >= pstate->maxLength)
	{
		//userdata->fileName = to_string(rand()) + ".avi";
		//userdata->fileHandle.open(userdata->fileName, CV_FOURCC('D','I','V','X'), 3, userdata->image.size(), false);
		//userdata->framesRecorded = 0;
		//cout<<"Long enough to create a new file\n";
	}


	mmal_buffer_header_mem_unlock(buffer);

	if (vcos_semaphore_trywait(&(userdata->complete_semaphore)) != VCOS_SUCCESS) {
		vcos_semaphore_post(&(userdata->complete_semaphore));
	}

	mmal_buffer_header_release(buffer);

	// and send one back to the port (if still open)
	if (port->is_enabled) {
		MMAL_STATUS_T status;

		new_buffer = mmal_queue_get(pstate->video_pool->queue);

		if (new_buffer)
			status = mmal_port_send_buffer(port, new_buffer);

		if (!new_buffer || status != MMAL_SUCCESS)
			printf("Unable to return a buffer to the video port\n");
	}
}

/**
 * Create the camera component, set up its ports
 *
 * @param state Pointer to state control struct
 *
 * @return MMAL_SUCCESS if all OK, something else otherwise
 *
 */
static MMAL_STATUS_T create_camera_component(RASPIVID_STATE *state)
{
   MMAL_COMPONENT_T *camera = 0;
   MMAL_ES_FORMAT_T *format;
   MMAL_PORT_T *preview_port = NULL, *video_port = NULL, *still_port = NULL;
   MMAL_STATUS_T status;

   /* Create the component */
   status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("Failed to create camera component");
      goto error;
   }

   if (!camera->output_num)
   {
      status = MMAL_ENOSYS;
      vcos_log_error("Camera doesn't have output ports");
      goto error;
   }

   preview_port = camera->output[MMAL_CAMERA_PREVIEW_PORT];
   video_port = camera->output[MMAL_CAMERA_VIDEO_PORT];
   still_port = camera->output[MMAL_CAMERA_CAPTURE_PORT];

   //  set up the camera configuration
   {
      MMAL_PARAMETER_CAMERA_CONFIG_T cam_config =
      {
         { MMAL_PARAMETER_CAMERA_CONFIG, sizeof(cam_config) },
         .max_stills_w = state->width,
         .max_stills_h = state->height,
         .stills_yuv422 = 0,
         .one_shot_stills = 0,
         .max_preview_video_w = state->width,
         .max_preview_video_h = state->height,
         .num_preview_video_frames = 3,
         .stills_capture_circular_buffer_height = 0,
         .fast_preview_resume = 0,
         .use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RESET_STC
      };
      mmal_port_parameter_set(camera->control, &cam_config.hdr);
   }

   // Now set up the port formats

   // Set the encode format on the Preview port
   // HW limitations mean we need the preview to be the same size as the required recorded output

   format = preview_port->format;
   format->encoding = MMAL_ENCODING_OPAQUE;
   format->encoding_variant = MMAL_ENCODING_I420;

   format->es->video.width = state->width;
   format->es->video.height = state->height;
   format->es->video.crop.x = 0;
   format->es->video.crop.y = 0;
   format->es->video.crop.width = state->width;
   format->es->video.crop.height = state->height;
   format->es->video.frame_rate.num = state->framerate;
   format->es->video.frame_rate.den = VIDEO_FRAME_RATE_DEN;

   status = mmal_port_format_commit(preview_port);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("camera viewfinder format couldn't be set");
      goto error;
   }

   // Set the encode format on the video  port

   format = video_port->format;
   format->encoding = MMAL_ENCODING_I420;
   format->encoding_variant = MMAL_ENCODING_I420;

   format->es->video.width = state->width;
   format->es->video.height = state->height;
   format->es->video.crop.x = 0;
   format->es->video.crop.y = 0;
   format->es->video.crop.width = state->width;
   format->es->video.crop.height = state->height;
   format->es->video.frame_rate.num = state->framerate;
   format->es->video.frame_rate.den = VIDEO_FRAME_RATE_DEN;

   video_port->buffer_size = state->width * state->height * 12 / 8;
   video_port->buffer_num = 1;
   //printf("  Camera video buffer_size = %d\n", video_port->buffer_size);

   status = mmal_port_format_commit(video_port);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("camera video format couldn't be set");
      goto error;
   }

   // Set the encode format on the still  port

   format = still_port->format;

   format->encoding = MMAL_ENCODING_OPAQUE;
   format->encoding_variant = MMAL_ENCODING_I420;
   format->es->video.width = state->width;
   format->es->video.height = state->height;
   format->es->video.crop.x = 0;
   format->es->video.crop.y = 0;
   format->es->video.crop.width = state->width;
   format->es->video.crop.height = state->height;
   format->es->video.frame_rate.num = 30;
   format->es->video.frame_rate.den = 1;

   status = mmal_port_format_commit(still_port);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("camera still format couldn't be set");
      goto error;
   }

   /* Ensure there are enough buffers to avoid dropping frames */
   if (still_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
      still_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;

   /* Enable component */
   status = mmal_component_enable(camera);

   if (status != MMAL_SUCCESS)
   {
      vcos_log_error("camera component couldn't be enabled");
      goto error;
   }

   // crate pool from camera still port
	if((state->video_pool = mmal_port_pool_create(video_port, video_port->buffer_num, video_port->buffer_size)) == NULL)
	{
		vcos_log_error("Error creating video pool\n");
		goto error;
	}

   raspicamcontrol_set_all_parameters(camera, &state->camera_parameters);

   state->camera_component = camera;

   if (state->verbose)
      fprintf(stderr, "Camera component done\n");

   return status;

error:

   if (camera)
      mmal_component_destroy(camera);

   return status;
}

/**
 * Destroy the camera component
 *
 * @param state Pointer to state control struct
 *
 */
static void destroy_camera_component(RASPIVID_STATE *state)
{
   if (state->camera_component)
   {
      mmal_component_destroy(state->camera_component);
      state->camera_component = NULL;
   }
}

/**
 * Connect two specific ports together
 *
 * @param output_port Pointer the output port
 * @param input_port Pointer the input port
 * @param Pointer to a mmal connection pointer, reassigned if function successful
 * @return Returns a MMAL_STATUS_T giving result of operation
 *
 */
static MMAL_STATUS_T connect_ports(MMAL_PORT_T *output_port, MMAL_PORT_T *input_port, MMAL_CONNECTION_T **connection)
{
   MMAL_STATUS_T status;

   status =  mmal_connection_create(connection, output_port, input_port, MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT);

   if (status == MMAL_SUCCESS)
   {
      status =  mmal_connection_enable(*connection);
      if (status != MMAL_SUCCESS)
         mmal_connection_destroy(*connection);
   }

   return status;
}

/**
 * Checks if specified port is valid and enabled, then disables it
 *
 * @param port  Pointer the port
 *
 */
static void check_disable_port(MMAL_PORT_T *port)
{
   if (port && port->is_enabled)
      mmal_port_disable(port);
}

/**
 * Set the camera configuration
 *
 * @param camera  Pointer the camera component
 * @param params  Parameters to be sent to the camera
 *
 */
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

/**
 * main
 */
int main(int argc, const char **argv)
{
   // Our main data storage vessel..
   RASPIVID_STATE state;

   MMAL_STATUS_T status = MMAL_SUCCESS;
   MMAL_PORT_T *camera_preview_port = NULL;
   MMAL_PORT_T *camera_video_port = NULL;
   MMAL_PORT_T *camera_still_port = NULL;
   MMAL_PORT_T *preview_input_port = NULL;

   bcm_host_init();

   // Register our application with the logging system
   vcos_log_register("RaspiVid", VCOS_LOG_CATEGORY);

   default_status(&state);

   // OK, we have a nice set of parameters. Now set up our components
   // We have two components. Camera and Preview
   
   if ((status = create_camera_component(&state)) != MMAL_SUCCESS)
   {
      vcos_log_error("%s: Failed to create camera component", __func__);
   }
   else if ((status = raspipreview_create(&state.preview_parameters)) != MMAL_SUCCESS)
   {
      vcos_log_error("%s: Failed to create preview component", __func__);
      destroy_camera_component(&state);
   }
   else
   {
      PORT_USERDATA callback_data;

	  srand (time(NULL));

	  //size of the image processed by the OpenCV
		float scale = 3;
		state.opencv_width = 1280 / scale;
		state.opencv_height = 720 / scale;

		float scale_width = 1280 / state.opencv_width;
		float scale_height = 720 / state.opencv_height;

		/* setup opencv */
		callback_data.image = Mat(Size(state.width, state.height), CV_8UC1);
		callback_data.image2 = Mat(Size(state.opencv_width, state.opencv_height), CV_8UC1);

		//set up video file
		callback_data.fileName = to_string(rand()) + ".avi";
		VideoWriter record(callback_data.fileName, CV_FOURCC('D','I','V','X'), 3, callback_data.image.size(), false);
		VideoWriter record2("fore_"+callback_data.fileName, CV_FOURCC('D','I','V','X'), 3, callback_data.image.size(), false);
		callback_data.fileHandle = record;
		callback_data.fileHandle2 = record2;

      if (state.verbose)
         fprintf(stderr, "Starting component connection stage\n");

      camera_preview_port = state.camera_component->output[MMAL_CAMERA_PREVIEW_PORT];
      camera_video_port   = state.camera_component->output[MMAL_CAMERA_VIDEO_PORT];
      camera_still_port   = state.camera_component->output[MMAL_CAMERA_CAPTURE_PORT];
      preview_input_port  = state.preview_parameters.preview_component->input[0];

		RASPICAM_CAMERA_PARAMETERS paramsCamera;

		paramsCamera.sharpness = 0;
		paramsCamera.contrast = 0;
		paramsCamera.brightness = 50;
		paramsCamera.saturation = 0;
		paramsCamera.ISO = 400;
		paramsCamera.videoStabilisation = 0;
		paramsCamera.exposureCompensation = 0;
		paramsCamera.exposureMode = MMAL_PARAM_EXPOSUREMODE_AUTO;
		paramsCamera.exposureMeterMode = MMAL_PARAM_EXPOSUREMETERINGMODE_AVERAGE;
		paramsCamera.awbMode = MMAL_PARAM_AWBMODE_OFF;
		paramsCamera.imageEffect = MMAL_PARAM_IMAGEFX_NONE;
		paramsCamera.colourEffects.enable = 0;
		paramsCamera.colourEffects.u = 128;
		paramsCamera.colourEffects.v = 128;
		paramsCamera.rotation = 180;
		paramsCamera.hflip = paramsCamera.vflip = 0;

		set_camera_parameters(state.camera_component, paramsCamera);

      if (state.preview_parameters.wantPreview )
      {
         if (state.verbose)
         {
            fprintf(stderr, "Connecting camera preview port to preview input port\n");
            fprintf(stderr, "Starting video preview\n");
         }

         // Connect camera to preview
        // status = connect_ports(camera_preview_port, preview_input_port, &state.preview_connection);
      }
      else
      {
         status = MMAL_SUCCESS;
      }

      if (status == MMAL_SUCCESS)
      {
         // Set up our userdata - this is passed though to the callback where we need the information.
         callback_data.pstate = &state;
         callback_data.abort = 0;
		 callback_data.humanDetected = 0;
		 callback_data.noHumanDetected = 0;
		 callback_data.uploading = false;
		 callback_data.framesRecorded = 0;

		 camera_video_port->userdata = (struct MMAL_PORT_USERDATA_T *)&callback_data;

		 // Enable the still port and tell it its callback function
         status = mmal_port_enable(camera_video_port, camera_control_callback);

         if (status != MMAL_SUCCESS)
         {
            vcos_log_error("Failed to setup video");
            goto error;
         }

		// Send all the buffers to the camera still port
		{
			int num = mmal_queue_length(state.video_pool->queue);
			int q;

			for (q = 0; q < num; q++)
			{
				MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(state.video_pool->queue);

				if (!buffer) {
					printf("Unable to get a required buffer %d from pool queue\n", q);
				}

				if (mmal_port_send_buffer(camera_video_port, buffer) != MMAL_SUCCESS) {
					printf("Unable to send a buffer to video port (%d)\n", q);
				}
			}
		}

			
        if (state.verbose)
            fprintf(stderr, "Starting video capture\n");

		if (mmal_port_parameter_set_boolean(camera_video_port, MMAL_PARAMETER_CAPTURE, 1) != MMAL_SUCCESS)
        {
            goto error;
        }

		vcos_semaphore_create(&callback_data.complete_semaphore, "mmal_opencv_demo-sem", 0);
		int opencv_frames = 0;

		//namedWindow("Display window", CV_WINDOW_AUTOSIZE);
		//namedWindow("Display window2", CV_WINDOW_AUTOSIZE);

		//Background subtraction variables
		/*const int varThreshold = 9;
		const bool bShadowDetection = true;
		const int history = 0;
		BackgroundSubtractorMOG2 bg(history,varThreshold,bShadowDetection);*/
		BackgroundSubtractorMOG2 bg;
		bg.set("detectShadows", true);

		//Configuration params from the config file
		int dilate_n = 5;						//Number of Dialtes
		int erode_n = 1;						//Number of Erodes
		state.maxLength = 900;					//10 minutes of video approximately (seconds * 3)
		state.consecutiveHumans = 1;
		state.consecutiveNoHumans = 45;			//15 seconds without human on the scene (seconds * 3)

		//Variables of excution
		bool firstRead = false;
		bool exposure = false;

		//system("/bin/bash /home/pi/build5/start.sh");

		while (1) {
			if (vcos_semaphore_wait(&(callback_data.complete_semaphore)) == VCOS_SUCCESS) {
				opencv_frames++;

				//Turn off exposure after the inicialization
				if(opencv_frames == 2 && exposure == false )
				{
					paramsCamera.exposureMode = MMAL_PARAM_EXPOSUREMODE_OFF;
					set_camera_parameters(state.camera_component, paramsCamera);
					exposure = true;
				}

				//Read configuration file
				if(opencv_frames >= OPENCV_CONFIG_FRAMES || firstRead == false)
				{
					string line="";
					ifstream file;

					file.open("config");
					if (!file.is_open())
					{
						cout<<"Error opening configuration file\n";
					}

					while(!file.eof())
					{
						string parameter;
						string value;

						getline(file, line);
						if(line.length()>=3)
						{
							parameter = line.substr(0,line.find('='));
							value = line.substr(line.find('=')+1);
							//cout<<"parameter:"<<parameter<<endl;
							//cout<<"value:"<<value<<endl;
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
							else if(parameter == "maxLength")
							{
								int aux;
								istringstream ( value ) >> aux;
								state.maxLength=aux;
							}
							else if(parameter == "consecutiveHumans")
							{
								int aux;
								istringstream ( value ) >> aux;
								state.consecutiveHumans=aux;
							}
							else if(parameter == "consecutiveNoHumans")
							{
								int aux;
								istringstream ( value ) >> aux;
								state.consecutiveNoHumans=aux;
							}
						}

					}
					file.close();
					if(!firstRead)
						firstRead = true;
					else
						opencv_frames = 0;
				}

				//Resizes the userdata.image and saves on userdata.image2 with the size of userdata.image2
				resize(callback_data.image, callback_data.image2, callback_data.image2.size(), 0, 0, CV_INTER_LINEAR);

				//Subtract and find the contours
				Mat fore;
				bg.operator() (callback_data.image2,fore);

				erode(fore, fore, Mat(), Point(-1,-1), erode_n);
				dilate(fore, fore, Mat(), Point(-1,-1), dilate_n);

				callback_data.fileHandle2 << fore;

				//imshow( "Display window", fore);

				vector<vector<cv::Point> > contours;

				findContours( fore, // binary input image 
								contours, // vector of vectors of points
								CV_RETR_EXTERNAL, // retrieve only external contours
								CV_CHAIN_APPROX_SIMPLE); // aproximate contours
			
				//Verify if the contour is a human
				int human = 0;
				for(int i=0;i < contours.size();i++)
				{
					vector<cv::Point> aux;

					approxPolyDP(contours[i], aux, 2, true);
					double arcLenght = arcLength(aux, true);
					double area = contourArea(contours[i]);
					Rect body_i = boundingRect(contours[i]);

					if(arcLenght > 100 && arcLenght < 1440)
					{
						cout<< (area/(body_i.width*body_i.height))*100 <<"\n";
						//check if the height is 2 times the width
						if(body_i.height>2*body_i.width)
						{
							human++;
							rectangle(callback_data.image2, 
										body_i, 
										Scalar(0,255,255), 
										3 
										);
						}
						//check if the contour area is bigger than 80% of the rectangle area
						//(supposition that can distinguish the diference between an animal and a group of people)
						else if(area>(0.8*body_i.width*body_i.height))
						{
							human++;
							rectangle(callback_data.image2, 
										body_i, 
										Scalar(0,255,255), 
										3 
										);
						}
					}
				}

				//Add human detected
				if(human > 0)
				{
					callback_data.humanDetected++;
					callback_data.noHumanDetected = 0;

					cout<<"Humans detected:"<<humanDetected;
				} 
				//Add human not detect
				else
				{
					callback_data.humanDetected = 0;
					callback_data.noHumanDetected++;
				}

				//Generate video in low quality with the square arround the object.
				resize(callback_data.image2, callback_data.image, callback_data.image.size(), 0, 0, CV_INTER_LINEAR);
				callback_data.fileHandle2 << callback_data.image;

				//imshow( "Display window2", callback_data.image2);
			
				//char key = (char) waitKey(1);
			}
        }
      }
      else
      {
         //mmal_status_to_int(status);
         vcos_log_error("%s: Failed to connect camera to preview", __func__);
      }

error:

      //mmal_status_to_int(status);

      if (state.verbose)
         fprintf(stderr, "Closing down\n");
		 
      // Disable all our ports that are not handled by connections
      check_disable_port(camera_still_port);

      if (state.preview_parameters.wantPreview )
         mmal_connection_destroy(state.preview_connection);

      /* Disable components */
      if (state.preview_parameters.preview_component)
         mmal_component_disable(state.preview_parameters.preview_component);

      if (state.camera_component)
         mmal_component_disable(state.camera_component);

      raspipreview_destroy(&state.preview_parameters);
      destroy_camera_component(&state);

      if (state.verbose)
         fprintf(stderr, "Close down completed, all components disconnected, disabled and destroyed\n\n");
   }

   if (status != MMAL_SUCCESS)
      raspicamcontrol_check_configuration(128);

   return 0;
}