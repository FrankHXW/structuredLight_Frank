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
//#define USE_UI_2220SE
#define USE_CANON_400D


using namespace std;
using namespace cv;

#ifdef USE_MV_UB500	//use mv-ub500 sdk api function
#include <process.h>
#include "windows.h"
#pragma comment(lib,"..\\structuredLight\\lib\\MVCAMSDK.lib")
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
INT redGain = 0, greenGain = 0, blueGain = 0; //0-100
INT gamma = 160; //multipe by 100
INT triggerMode = IS_SET_TRIGGER_SOFTWARE; // IS_SET_TRIGGER_LO_HI;
INT triggerDelay = 0;
INT flashMode = IO_FLASH_MODE_TRIGGER_LO_ACTIVE;	//pay attention to that real output is inverse
char *imageAddress = NULL;
INT memoryId = 0;


#elif defined(USE_CANON_400D)
#include <process.h>
#include "EDSDK.h"
#include "EDSDKTypes.h"
#include "EDSDKErrors.h"

EdsCameraRef canonCamera;
EdsPropertyID canonPropertyID;
EdsPropertyDesc propertyModifiable;

//following shooting-related properties are all EdsUInt32 
EdsUInt32 edsAEMode = 0;			//Shooting mode: 0-P, 1-Tv, 2-Av, 3-M,
EdsUInt32 edsDriveMode = 0x0;		//Drive mode: 0x0-single frame shooting, 0x1-continuous shooting
EdsUInt32 edsISOSpeed = 0x48;		//ISO speed: 0x48-ISO100, 0x50-ISO200, 0x58-ISO400, 0x60-ISO800, 0x68-ISO1600
EdsUInt32 edsMeteringMode = 3;		//Metering mode: 3-Evaluative, 4-Parital, 5-Center weighted
EdsUInt32 edsAFMode = 0;			//AFMode: 0-One-Shot, 1-AI-Servo, 2-AI-Focus, 3-Manual
EdsUInt32 edsApertureValue = 0x30;	//Aperture value: 0x25-3.5, 0x28-4, 0x2B-4.5, 0x2D-5.0, 0x30-5.6, 0x33-6.3, 0x35-7.1
EdsUInt32 edsShutterValue = 0x5d;	//Shutter speed: 0x4b-1/5, 0x4d-1/6, 0x50-1/8, 0x53-1/10, 0x58-1/15, 0x5b-20, 0x5d-1/25, 0x60-1/30, 0x63-1/40, 0x64-1/45, 0x65-1/50   
EdsUInt32 edsExposureCompensation = 0x0;	//Exposure compensation: 0x10-2,0x08-1, 0x0-0, 0xf8--1, 0xf0--2
EdsUInt32 edsFlashCompensation = 0x0;		//Flash compensation: 0x10-2,0x08-1, 0x0-0, 0xf8--1, 0xf0--2

//following Image Properties are all EdsUInt32
EdsUInt32 edsImageQuality = 0x02130f0f;		//Image quality: 0x02130f0f-small, 0x01130f0f-medium, 0x00130f0f-large
EdsUInt32 edsWhiteBalance = 1;				//White balance: 0-AWB, 1-daylight, 2-cloudy, 3-tungsten, 4-fluorscent, 5-flash 
EdsUInt32 edsSaveTo = 2;					//destination of images: 1-memeory card, 2-host computer, 3-both
string imageSaveDirectory = "output\\canon";	//image save directory

//canon sdk's exclusively thread id,create by main thread
unsigned int EDSDKThreadID;
bool edsPhotoTaken = false;
#define EDSDK_MESSAGE_TAKEPICTURE WM_USER+1
#define EDSDK_MESSAGE_EXIT		  WM_USER+2


//define canon download image
void CanonDownloadImage(EdsDirectoryItemRef dirItemRef, string directory){

	cout << "begin to download image" << endl;
	EdsError error;
	EdsDirectoryItemInfo dirItemInfo;
	EdsStreamRef streamRef = NULL;
	//get information about object
	error = EdsGetDirectoryItemInfo(dirItemRef, &dirItemInfo);
	string savePath = directory + "\\" + "image_400d.jpg";   //pay attention to the ".jpg"

	
	//create filestream to data
	if (error == EDS_ERR_OK){
		error = EdsCreateFileStream(savePath.c_str(), kEdsFileCreateDisposition_CreateAlways, kEdsAccess_ReadWrite, &streamRef);
	}
	//clock_t clock_begin = clock();
	//unsigned char userBuffer[1030000];
	//EdsCreateMemoryStreamFromPointer(userBuffer,1030000,&streamRef);
	//download file
	if (error == EDS_ERR_OK){
		error=EdsDownload(dirItemRef, dirItemInfo.size, streamRef);
	}

	//Mat tmp(1288, 1936, CV_8UC3, Scalar(0));
	//vector<unsigned char> userBuffer_vec(1030000);
	//for (int i = 0; i < 1030000; ++i)
	//	userBuffer_vec[i]=userBuffer[i];
	//tmp = imdecode(userBuffer_vec,1);
	//cout << clock() - clock_begin << endl;
	if (error == EDS_ERR_OK){
		EdsDownloadComplete(dirItemRef);
		EdsRelease(dirItemRef);
		EdsRelease(streamRef);
		edsPhotoTaken = true;
	}
}

//define event callback function
EdsError EDSCALLBACK handleObjectEvent(EdsObjectEvent event, EdsBaseRef object, EdsVoid *context)
{
	switch (event){
	case kEdsObjectEvent_All:
		break;
	case kEdsObjectEvent_DirItemCancelTransferDT:
		break;
	case kEdsObjectEvent_DirItemContentChanged:
		break;
	case kEdsObjectEvent_DirItemCreated:
		break;
	case kEdsObjectEvent_DirItemInfoChanged:
		break;
	case kEdsObjectEvent_DirItemRemoved:
		break;
	case kEdsObjectEvent_DirItemRequestTransfer:
		CanonDownloadImage(object, imageSaveDirectory);
		break;
	case kEdsObjectEvent_DirItemRequestTransferDT:
		break;
	case kEdsObjectEvent_FolderUpdateItems:
		break;
	case kEdsObjectEvent_VolumeAdded:
		break;
	case kEdsObjectEvent_VolumeInfoChanged:
		break;
	case kEdsObjectEvent_VolumeRemoved:
		break;
	case kEdsObjectEvent_VolumeUpdateItems:
		break;
	}

	//object must be released
	if (object){
		EdsRelease(object);
	}
	return EDS_ERR_OK;
}

//a exclusively thread used for EDSDK
unsigned __stdcall EDSDKMessageLoop(void * i){
	EdsError	 error = EDS_ERR_OK;
	EdsCameraListRef cameraList = NULL;
	EdsUInt32	 count = 0;
	bool		 isSDKLoaded = false;

	// Initialization of SDK
	error = EdsInitializeSDK();
	if (error == EDS_ERR_OK){
		isSDKLoaded = true;
	}
	//Acquisition of camera list
	if (error == EDS_ERR_OK){
		error = EdsGetCameraList(&cameraList);
	}
	//Acquisition of number of Cameras
	if (error == EDS_ERR_OK){
		error = EdsGetChildCount(cameraList, &count);
		if (count == 0){
			error = EDS_ERR_DEVICE_NOT_FOUND;
		}
	}
	//Acquisition of camera at the head of the list
	if (error == EDS_ERR_OK){
		error = EdsGetChildAtIndex(cameraList, 0, &canonCamera);
	}
	//Acquisition of camera information
	EdsDeviceInfo deviceInfo;
	if (error == EDS_ERR_OK){
		error = EdsGetDeviceInfo(canonCamera, &deviceInfo);
		if (error == EDS_ERR_OK && canonCamera == NULL){
			error = EDS_ERR_DEVICE_NOT_FOUND;
		}
	}
	else
		cout << "Cannot detect camera" << endl;
	//Release camera list
	if (cameraList != NULL){
		EdsRelease(cameraList);
	}
	//subscribe to the camera events
	EdsSetObjectEventHandler(canonCamera, kEdsObjectEvent_All, handleObjectEvent, NULL);

	//The communication with the camera begins
	error = EdsOpenSession(canonCamera);
	EdsDataType	dataType = kEdsDataType_Unknown;
	EdsUInt32   dataSize = 0;
	EdsUInt32 data = 0;

	//get the AEMode and remind user to set the mode 
	error = EdsGetPropertyData(canonCamera, kEdsPropID_AEMode, 0, sizeof(edsAEMode), &edsAEMode);
	if (error == EDS_ERR_OK){
		if (edsAEMode != 3){		//current AEMode is not manual exposure
			cout << "please set canon camera to manual mode!" << endl;
			return -1;
		}
	}
	//set the Drive mode
	EdsGetPropertyDesc(canonCamera, kEdsPropID_DriveMode, &propertyModifiable);
	error = EdsSetPropertyData(canonCamera, kEdsPropID_DriveMode, 0, sizeof(edsDriveMode), &edsDriveMode);
	if (error == !EDS_ERR_OK){
		cout << "set Drive mode at " << edsDriveMode << "failed" << endl;
		return -1;
	}
	//set the ISOSpeed 
	EdsGetPropertyDesc(canonCamera, kEdsPropID_ISOSpeed, &propertyModifiable);
	error = EdsSetPropertyData(canonCamera, kEdsPropID_ISOSpeed, 0, sizeof(edsISOSpeed), &edsISOSpeed);
	if (error == !EDS_ERR_OK){
		cout << "set ISOSpeed at " << edsISOSpeed << "failed" << endl;
		return -1;
	}
	//set the MeteringMode 
	EdsGetPropertyDesc(canonCamera, kEdsPropID_MeteringMode, &propertyModifiable);
	error = EdsGetPropertyData(canonCamera, kEdsPropID_MeteringMode, 0, sizeof(edsMeteringMode), &edsMeteringMode);
	if (error == !EDS_ERR_OK){
		cout << "set MeteringMode at " << edsMeteringMode << "failed" << endl;
		return -1;
	}
	//set the AFMode 
	EdsGetPropertyDesc(canonCamera, kEdsPropID_AFMode, &propertyModifiable);
	error = EdsGetPropertyData(canonCamera, kEdsPropID_AFMode, 0, sizeof(edsAFMode), &edsAFMode);
	if (error == EDS_ERR_OK){
		if (edsAFMode != 3){	//it is not in maual AFMode
			cout << "please set AFMode to manual!" << endl;
			return -1;
		}
	}
	//set thr Aperture value
	EdsGetPropertyDesc(canonCamera, kEdsPropID_Av, &propertyModifiable);
	error = EdsSetPropertyData(canonCamera, kEdsPropID_Av, 0, sizeof(edsApertureValue), &edsApertureValue);
	if (error == !EDS_ERR_OK){
		cout << "set Aperture value at " << edsApertureValue << "failed" << endl;
		return -1;
	}
	//set the Shutter speed
	EdsGetPropertyDesc(canonCamera, kEdsPropID_Tv, &propertyModifiable);
	error = EdsSetPropertyData(canonCamera, kEdsPropID_Tv, 0, sizeof(edsShutterValue), &edsShutterValue);
	if (error == !EDS_ERR_OK){
		cout << "set Shutter speed at " << edsShutterValue << "failed" << endl;
		return -1;
	}
	//set the Exposure compensation
	EdsGetPropertyDesc(canonCamera, kEdsPropID_ExposureCompensation, &propertyModifiable);
	error = EdsSetPropertyData(canonCamera, kEdsPropID_ExposureCompensation, 0, sizeof(edsExposureCompensation), &edsExposureCompensation);
	if (error == !EDS_ERR_OK){
		cout << "set Exposure compensation at " << edsExposureCompensation << "failed" << endl;
		return -1;
	}
	//set the Flash Compensation
	EdsGetPropertyDesc(canonCamera, kEdsPropID_FlashCompensation, &propertyModifiable);
	error = EdsSetPropertyData(canonCamera, kEdsPropID_FlashCompensation, 0, sizeof(edsFlashCompensation), &edsFlashCompensation);
	if (error == !EDS_ERR_OK){
		cout << "set FlashCompensation at " << edsFlashCompensation << "failed" << endl;
		return -1;
	}
	//set the Image quality
	EdsGetPropertyDesc(canonCamera, kEdsPropID_ImageQuality, &propertyModifiable);
	error = EdsSetPropertyData(canonCamera, kEdsPropID_ImageQuality, 0, sizeof(edsImageQuality), &edsImageQuality);
	if (error == !EDS_ERR_OK){
		cout << "set Image quality at " << edsImageQuality << "failed" << endl;
		return -1;
	}
	//set the White balance
	EdsGetPropertyDesc(canonCamera, kEdsPropID_WhiteBalance, &propertyModifiable);
	error = EdsSetPropertyData(canonCamera, kEdsPropID_WhiteBalance, 0, sizeof(edsWhiteBalance), &edsWhiteBalance);
	if (error == !EDS_ERR_OK){
		cout << "set White balance at " << edsWhiteBalance << "failed" << endl;
		return -1;
	}
	//set the Save to destination
	EdsGetPropertyDesc(canonCamera, kEdsPropID_SaveTo, &propertyModifiable);
	error = EdsSetPropertyData(canonCamera, kEdsPropID_SaveTo, 0, sizeof(edsSaveTo), &edsSaveTo);
	if (error == !EDS_ERR_OK){
		cout << "set Save to " << edsSaveTo << "failed" << endl;
		return -1;
	}

	MSG msg;
	while(GetMessage(&msg, NULL, NULL, NULL)>0)
	{
		switch (msg.message){
			case EDSDK_MESSAGE_TAKEPICTURE:
				EdsSendCommand(canonCamera, kEdsCameraCommand_TakePicture, 0);
				edsPhotoTaken = false;
//				cout << "get taking picture command" << endl;
				break;
			case EDSDK_MESSAGE_EXIT:
//				cout << "clear camera" << endl;
				EdsCloseSession(canonCamera);
				EdsRelease(canonCamera);
				EdsTerminateSDK();
				break;
			default:
				TranslateMessage(&msg);
				DispatchMessage(&msg);
//				cout << "this is MessageLoop" << endl;
				break;
		}
	}
	return 0;
}


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

#elif defined(USE_CANON_400D)
	uintptr_t handleEDSDK;
	//create a exclusively thread for EDSDK
	handleEDSDK=_beginthreadex(NULL, 10000000, EDSDKMessageLoop, NULL, 0, &EDSDKThreadID);
	if (handleEDSDK) return -1;
	return 0;

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
	is_FreezeVideo(hCam, IS_WAIT);
	frame_grab.data = (unsigned char *)imageAddress;	//pay attenion to this,there is no new allocated data
	return 0;

#elif defined(USE_CANON_400D)
	PostThreadMessage(EDSDKThreadID, EDSDK_MESSAGE_TAKEPICTURE, 0, 0);
	while (edsPhotoTaken ){
		clock_t clock_begin = clock();
		frame_grab=imread("output\\canon\\image_400d.jpg");
		imshow("EDSDK", frame_grab);
		cout << clock() - clock_begin;
		break;
	}
	return 0 ;
#endif
}

void CameraClear(void)
{
#ifdef USE_MV_UB500		//use mv-ub500 sdk api function
	CameraUnInit(m_hCamera);

#elif defined(USE_UI_2220SE)	//use UI-2220SE camera sdk api function
	is_ExitCamera (hCam);

#elif defined(USE_CANON_400D)
	PostThreadMessage(EDSDKThreadID, EDSDK_MESSAGE_EXIT, 0, 0);
#endif
}

#ifdef USE_UI_2220SE
void UI_2220SE_capture(){
	Mat tmp(slparameter.camera_height, slparameter.camera_width, CV_8UC3, Scalar(0));
	vector<Mat> imageCaptured(22);
	for (int i = 0; i < 22; ++i)
		imageCaptured[i] = Mat(slparameter.camera_height, slparameter.camera_width, CV_8UC3, Scalar(0));
	while (1){
		if (LCR_PatternDisplay(2) >= 0)			//Start Pattern Display Sequence
			for (unsigned int i = 6000000; i > 0; i--);	//there must be an delay >10ms

		unsigned int step = tmp.cols*tmp.rows;
		for (int i = 0; i < 22; ++i){
			clock_begin = clock();
			GetImage(tmp);
			unsigned char *ptr_src = tmp.ptr <unsigned char>(0);
			unsigned char *ptr_dst = imageCaptured[i].ptr<unsigned char>(0);
			for (int i = 0; i < 3 * step; ++i)
				ptr_dst[i] = ptr_src[i];
			cout << clock() - clock_begin << " :" << i << endl;
		}
		clock_begin = clock();
		//		if(LCR_PatternDisplay(0)>=0)			//Stop Pattern Display Sequence
		//			for (unsigned int i = 80000000; i > 0; i--);	//there must be an delay >10ms
		cout << clock() - clock_begin << endl;
	}
}
#endif





