#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <direct.h>
#include <time.h>

#include "structuredlight.h"
#include "camera.h"


//#define USE_MV_UB500
#define USE_UI_2220SE

using namespace std;
using namespace cv;

#ifdef USE_MV_UB500	//use mv-ub500 sdk api function
#include <process.h>
#include "windows.h"
#pragma comment(lib,"..\\structuredLight\\MVCAMSDK.lib")
#include "CameraApi.h"

CameraSdkStatus camera_sdk_status;	//相机状态
INT  CameraNums = 1;				//设置iCameraNums = 1，表示最多只读取1个设备，如需枚举更多的设备，更改sCameraList大小和iCameraNums的值
tSdkCameraDevInfo CameraList[1];	//设备列表数组指针
CameraHandle    m_hCamera;			//相机句柄，多个相机同时使用时，可以用数组代替	
tSdkCameraCapbility CameraInfo;		//相机特性描述结构体
tSdkFrameHead 	FrameInfo;			//图像的帧头信息指针
BYTE*			PbyBuffer;			//指向原始图像数据的缓冲区指针
BYTE*           FrameBuffer;		//将原始图像数据转换为RGB图像的缓冲区
IplImage *iplImage = NULL;


#elif defined(USE_UI_2220SE)	//use UI-2220SE camera sdk api function
#include "uEye.h"
HIDS hCam = 1;		//camera handle
SENSORINFO SensorInfo;
CAMINFO CameraInfo;
INT colorMode = IS_CM_BGR8_PACKED;
UINT pixelClock = 30; //MHz
double frameRate = 52;  //fps
double exposureTime = 8.0; //ms
INT  masterGain = 0;  //0-100
INT redGain = 5, greenGain = 0, blueGain = 60; //0-100
INT gamma = 160; //multipe by 100
INT triggerMode = IS_SET_TRIGGER_SOFTWARE; // IS_SET_TRIGGER_LO_HI;
INT triggerDelay = 0;
INT flashMode = IO_FLASH_MODE_TRIGGER_LO_ACTIVE;	//pay attention to that real output is inverse
char *imageAddress = NULL;
INT memoryId = 0;
#endif


//Initialize camera
int CameraInitialize(SlParameter &sl_parameter)
{
//	cv::Mat frame_grab;
	//use usb webcam
//	VideoCapture videocapture(sl_parameter.camera_id);
//	if (!videocapture.isOpened())
//	{
//		cerr << "Failed to open camera" << endl;
//		return -1;
//	}
//	cout << "Initialize camera------------------------------------------" << endl;
//	//set properties of the camera
//	videocapture.set(CV_CAP_PROP_FRAME_WIDTH, sl_parameter.camera_width);
//	videocapture.set(CV_CAP_PROP_FRAME_HEIGHT, sl_parameter.camera_height);
//
//	namedWindow("Video in real-time", WINDOW_NORMAL);
//	while (1)
//	{
//		videocapture >> frame_grab;
//		imshow("Video in real-time", frame_grab);
//		waitKey(50);
//#ifdef DEBUG_PROJECT
//		cout << "Camera Properties:" << endl;
//		cout << "camera id: " << sl_parameter.camera_id << endl;
//		cout << "frame rate: " << videocapture.get(CV_CAP_PROP_FPS) << endl;
//		cout << "width: " << videocapture.get(CV_CAP_PROP_FRAME_WIDTH) << endl;
//		cout << "height: " << videocapture.get(CV_CAP_PROP_FRAME_HEIGHT) << endl;
//		cout << "brightness: " << videocapture.get(CV_CAP_PROP_BRIGHTNESS) << endl;
//		cout << "contrast: " << videocapture.get(CV_CAP_PROP_CONTRAST) << endl;
//		cout << "saturation: " << videocapture.get(CV_CAP_PROP_SATURATION) << endl;
//		cout << "hue: " << videocapture.get(CV_CAP_PROP_HUE) << endl;
//		cout << "gain: " << videocapture.get(CV_CAP_PROP_GAIN) << endl;
//		cout << "exposure: " << videocapture.get(CV_CAP_PROP_EXPOSURE) << endl;
//#endif
//		cout << "-------------------------------------------------------" << endl << endl;
//	}

#ifdef USE_MV_UB500		//use mv-ub500 sdk api function
	//use industry camera mv ub500
	Mat frame_grab;
	//相机SDK初始化
	if ((camera_sdk_status= CameraSdkInit(1)) != CAMERA_STATUS_SUCCESS)
	{
		cout << "Camera sdk init failed: " << camera_sdk_status<<endl;
		return -1;
	}
	//枚举设备，获得设备列表
	if ((camera_sdk_status=CameraEnumerateDevice(CameraList, &CameraNums)) != CAMERA_STATUS_SUCCESS || CameraNums == 0)
	{
		cout << "No camera was found: " << camera_sdk_status << endl;
		return -1;
	}
	//初始化设备
	if ((camera_sdk_status  = CameraInit(&CameraList[0], -1, -1, &m_hCamera)) != CAMERA_STATUS_SUCCESS)
	{
		cout << "Camera  init failed: " << camera_sdk_status << endl;
		return -1;
	}
	//初始化相机的特性描述
	CameraGetCapability(m_hCamera, &CameraInfo);
	FrameBuffer = (BYTE *)malloc(CameraInfo.sResolutionRange.iWidthMax*CameraInfo.sResolutionRange.iWidthMax * 3);
	//进入图像采集模式
	CameraPlay(m_hCamera);
	waitKey(2000);			//wait for camera start
	//for (int i = 0; i < 5; i++)
	//{
	//	GetImage(frame_grab);
	//	if (!frame_grab.empty())
	//		imshow("camera initialize", frame_grab);
	//	waitKey(200);
	//}
	GetImage(frame_grab);
	if (!frame_grab.empty())
		cout << "Camera initialize successful......" <<endl<<endl;
	else
		cout << "Camera initializa failed......" << endl<<endl;
//	destroyWindow("camera initialize");
	return 0;

#elif defined(USE_UI_2220SE)	//use UI-2220SE camera sdk api function
	
	INT nRet = is_InitCamera(&hCam, NULL);
	if (nRet == IS_SUCCESS){
		cout << "camera UI_2220SE init success!" << endl;

		//query and display information about sensor and camera
		is_GetSensorInfo(hCam, &SensorInfo);
		is_GetCameraInfo(hCam, &CameraInfo);
		
		nRet = is_SetColorMode(hCam, colorMode);
		if (nRet != IS_SUCCESS){
			cout << "set color mode at" << colorMode << "failed;error code:" << nRet << endl;
		}

		//setting parameters,closing all automatic function firstly
		double auto_parameter1 = 0, auto_parameter2 = 0;

		nRet=is_PixelClock(hCam, IS_PIXELCLOCK_CMD_SET, (void*)&pixelClock, sizeof(pixelClock));
		if (nRet != IS_SUCCESS){
			cout << "set pixelclock at " << pixelClock << "MHz failed;error code:" << nRet << endl;
			return 0;
		}

		is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_SENSOR_FRAMERATE, &auto_parameter1, &auto_parameter2);
		nRet=is_SetFrameRate(hCam, frameRate, &frameRate);
		if (nRet != IS_SUCCESS){
			cout << "set framerate at " << frameRate << "fps failed;error code:" << nRet << endl;
			return 0;
		}

		is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_SENSOR_GAIN_SHUTTER, &auto_parameter1, &auto_parameter2);
		nRet = is_Exposure(hCam, IS_EXPOSURE_CMD_SET_EXPOSURE, &exposureTime, 8);
		if (nRet != IS_SUCCESS){
			cout << "set exposuretime at " << exposureTime << "ms failed;error code:" << nRet << endl;
			return 0;
		}

		is_SetAutoParameter(hCam, IS_SET_ENABLE_AUTO_SENSOR_GAIN, &auto_parameter1, &auto_parameter2);
		is_SetGainBoost(hCam, IS_SET_GAINBOOST_ON);
		nRet=is_SetHardwareGain(hCam, masterGain, redGain, greenGain, blueGain);
		if (nRet != IS_SUCCESS){
			cout << "set hardware gain failed;error code:" << nRet << endl;
			return 0;
		}

		is_SetHardwareGamma(hCam, IS_SET_HW_GAMMA_ON);
		nRet = is_Gamma(hCam, IS_GAMMA_CMD_SET, &gamma, sizeof(gamma));
		if (nRet != IS_SUCCESS){
			cout << "set hardware gamma failed;error code:" << nRet << endl;
			return 0;
		}

//		is_SetTriggerDelay(hCam, triggerDelay);
		nRet = is_SetExternalTrigger(hCam, triggerMode);
		if (nRet != IS_SUCCESS){
			cout << "set external trigger at " << triggerMode << "failed;error code:" << nRet << endl;
			return 0;
		}

		//must set trigger firstly
		IO_FLASH_PARAMS flashParams;
		flashParams.s32Delay = 0;	   //delay after explosure start(us)
		flashParams.u32Duration = 0;   //flash voltage level duration(us),0=explosure time
		is_IO(hCam, IS_IO_CMD_FLASH_SET_MODE, (void*)&flashMode, sizeof(flashMode));
		nRet = is_IO(hCam, IS_IO_CMD_FLASH_SET_PARAMS, (void*)&flashParams, sizeof(flashParams));
		if (nRet != IS_SUCCESS){
			cout << "set flash mode failed;error code:" << nRet << endl;
			return 0;
		}

		nRet=is_AllocImageMem(hCam, sl_parameter.camera_width, sl_parameter.camera_height, 24, &imageAddress, &memoryId);
		if (nRet == IS_SUCCESS){
			nRet = is_SetImageMem(hCam, imageAddress, memoryId);
			if (nRet != IS_SUCCESS){
				cout << "allocate memory failed;error code:" << nRet << endl;
				return 0;
			}
		}

		return -1;
	}
	else{
		cout << "Init camera UI-2220SE failed;error code:" << nRet<< endl;
		return 0;
	}
#endif
	
}

int GetImage(Mat &frame_grab)
{
#ifdef USE_MV_UB500		//use mv-ub500 sdk api function
	clock_t clock_begin;
	clock_begin = clock();
	//注意相机输出的是上一次触发捕获指令缓存的图像
	if (CameraGetImageBuffer(m_hCamera, &FrameInfo, &PbyBuffer, 200) == CAMERA_STATUS_SUCCESS)
	{
		////将获得的原始数据转换成RGB格式的数据，同时经过ISP模块，对图像进行降噪，边沿提升，颜色校正等处理。
		camera_sdk_status = CameraImageProcess(m_hCamera, PbyBuffer, FrameBuffer, &FrameInfo);//连续模式
		if (camera_sdk_status == CAMERA_STATUS_SUCCESS)
		{
			//转换数据并显示
			iplImage = cvCreateImageHeader(cvSize(FrameInfo.iWidth, FrameInfo.iHeight), IPL_DEPTH_8U, 3);
			cvSetData(iplImage, FrameBuffer, FrameInfo.iWidth * 3);
			//cvShowImage("camera", iplImage);
			Mat frame_temp(iplImage, true);
			frame_grab = frame_temp.clone();
		}
		//在成功调用CameraGetImageBuffer后，必须调用CameraReleaseImageBuffer来释放获得的buffer。
		CameraReleaseImageBuffer(m_hCamera, PbyBuffer);
		cvReleaseImageHeader(&iplImage);
		cout << clock() - clock_begin << " ";
		return -1;
	}
	else
		return 0;
#elif defined(USE_UI_2220SE) 	//use UI-2220SE camera sdk api function
	clock_t clock_begin;
	clock_begin = clock();
	is_FreezeVideo(hCam, IS_WAIT);
	cout << clock() - clock_begin << " ";

	frame_grab.data = (unsigned char *)imageAddress;
	return 0;
#endif

}

void CameraClear(void)
{
#ifdef USE_MV_UB500		//use mv-ub500 sdk api function
	CameraUnInit(m_hCamera);

#elif defined(USE_UI_2220SE)	//use UI-2220SE camera sdk api function
	is_ExitCamera (hCam);
#endif
}








