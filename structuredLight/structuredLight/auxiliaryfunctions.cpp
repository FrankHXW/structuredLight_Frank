#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <direct.h>

#include "structuredlight.h"
#include "auxiliaryfunctions.h"


using namespace std;
using namespace cv;
//use mv-ub500 sdk api function
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

//注意相机输出的是上一次触发捕获指令缓存的图像
int GetImage(Mat &frame_grab)
{
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
			frame_grab=frame_temp.clone();
		}
		//在成功调用CameraGetImageBuffer后，必须调用CameraReleaseImageBuffer来释放获得的buffer。
		CameraReleaseImageBuffer(m_hCamera, PbyBuffer);
		cvReleaseImageHeader(&iplImage);
		return -1;
	}
	else
		return 0;
}

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
		cout << "Camera initialize successful !" <<endl<<endl;
	else
		cout << "Camera initializa failed !" << endl<<endl;
//	destroyWindow("camera initialize");
	
	return 0;
}
void Clear(void)
{
	CameraUnInit(m_hCamera);
}


//Initialize projector
int ProjectorInitialize(SlParameter &sl_parameter)
{
	//show full white projector background pattern
	sl_parameter.projector_background_pattern = Mat::zeros(sl_parameter.projector_height, sl_parameter.projector_width, CV_8U);
	sl_parameter.projector_background_pattern.setTo(255);
	imshow("projector window", sl_parameter.projector_background_pattern);
	waitKey(1000);
	cout << "projector initialize successful!" << endl << endl;
	return 0;
}


//generate chessboard pattern 
int GenerateChessBoardPattern(SlParameter &sl_parameter)
{
	cout << "generate chessboard pattern......." << endl;
	int row_index, colum_index;
	int pixel_x, pixel_y;
	sl_parameter.projector_chessboard_pattern = Mat::zeros(sl_parameter.projector_height, sl_parameter.projector_width, CV_8U);
	sl_parameter.projector_chessboard_pattern.setTo(255);

	//calculate starting colum and row
	sl_parameter.projector_border_colum = (int)((sl_parameter.projector_width - (sl_parameter.projector_board_size.width + 1)*sl_parameter.projector_square_size.width) / 2);
	sl_parameter.projector_border_row = (int)((sl_parameter.projector_height - (sl_parameter.projector_board_size.height + 1)*sl_parameter.projector_square_size.height) / 2);

	if (sl_parameter.projector_border_colum < 0 || sl_parameter.projector_border_row < 0)
	{
		cout << "projector physical width and height:" << sl_parameter.projector_width << "X" << sl_parameter.projector_height << endl;
		cerr << "can't generate " << (sl_parameter.projector_board_size.width + 1)*sl_parameter.projector_square_size.width << "X" <<
			(sl_parameter.projector_board_size.height + 1)*sl_parameter.projector_square_size.height << "chessboard" << endl;
		return -1;
	}

	//create odd black square
	for (row_index = 0; row_index <sl_parameter.projector_board_size.height + 1; row_index += 2)
	{
		for (colum_index = 0; colum_index < sl_parameter.projector_board_size.width + 1; colum_index += 2)
		{
			for (pixel_y = 0; pixel_y < sl_parameter.projector_square_size.height; pixel_y++)
			{
				for (pixel_x = 0; pixel_x < sl_parameter.projector_square_size.width; pixel_x++)
					sl_parameter.projector_chessboard_pattern.at<unsigned char>(pixel_y + row_index*sl_parameter.projector_square_size.height + sl_parameter.projector_border_row,
					pixel_x + colum_index*sl_parameter.projector_square_size.width + 500) = 0;  //pay attention to where the chessboard start
			}
		}
	}
	//create even black square
	for (row_index = 1; row_index < sl_parameter.projector_board_size.height; row_index += 2)
	{
		for (colum_index = 1; colum_index < sl_parameter.projector_board_size.width; colum_index += 2)
		{
			for (pixel_y = 0; pixel_y < sl_parameter.projector_square_size.height; pixel_y++)
			{
				for (pixel_x = 0; pixel_x < sl_parameter.projector_square_size.width; pixel_x++)
					sl_parameter.projector_chessboard_pattern.at<unsigned char>(pixel_y + row_index*sl_parameter.projector_square_size.height + sl_parameter.projector_border_row,
					pixel_x + colum_index*sl_parameter.projector_square_size.width + 500) = 0; //pay attention to where the chessboard start
			}
		}
	}
	cout << "generate chessboard pattern successful......." << endl;
}




//Create output directory
int CreateOutputDirectory(SlParameter &sl_parameter)
{
	cout << "Create output directory......"<<endl;
	_mkdir(sl_parameter.output_directory.c_str()); //c_str:convert string to c_string(string's data plus'\0')
	_mkdir((sl_parameter.output_directory + "\\" + sl_parameter.object_name).c_str());
	//clean existing object_name directory and then recreate
	//string str ="rd /s /q \""+ sl_parameter.output_directory + "\\" + sl_parameter.object_name+"\"";
	//system(str.c_str());
	//if (_mkdir((sl_parameter.output_directory + "\\" + sl_parameter.object_name).c_str()) != 0)
	//{
	//	cerr << "Can't open " << sl_parameter.output_directory <<"\\"<<sl_parameter.object_name << endl;
	//	return -1;
	//}
	cout << "create output directory successful!" << endl<<endl;
	return 0;
}



//Read structuredlight setting parameters from structuredlight.xml
int Read_slparameter(SlParameter &sl_parameter, SlCalibration &sl_calibration)
{
	//Open configure file 
	FileStorage slparameter_file("slparameter.xml", FileStorage::READ);
	if (!slparameter_file.isOpened())
	{
		cerr << "Failed to open slparameter.xml!" << endl;
		return -1;
	}

	cout << "Reading configure parameters........" << endl;
	//Read output directory and object name
	sl_parameter.output_directory = slparameter_file["output"]["output_directory"];
	sl_parameter.object_name = slparameter_file["output"]["object_name"];
	sl_parameter.save_enable = (int)(slparameter_file["output"]["save_enable"]);
#ifdef DEBUG_PROJECT
	cout << "output:" << endl;
	cout << "output_directory: " << sl_parameter.output_directory << endl;
	cout << "object_name: " << sl_parameter.object_name << endl;
	cout << "save_enable: " << sl_parameter.save_enable << endl << endl;
#endif

	//Read camera parameters
	sl_parameter.camera_id = slparameter_file["camera"]["id"];
	sl_parameter.camera_width = slparameter_file["camera"]["width"];
	sl_parameter.camera_height = slparameter_file["camera"]["height"];
#ifdef DEBUG_PROJECT
	cout << "camera:" << endl;
	cout << "width: " << sl_parameter.camera_width << endl;
	cout << "height: " << sl_parameter.camera_height << endl << endl;
#endif

	//Read projector parameters
	sl_parameter.projector_width = slparameter_file["projector"]["width"];
	sl_parameter.projector_height = slparameter_file["projector"]["height"];
	sl_parameter.projector_invert = (int)slparameter_file["projector"]["invert"];
#ifdef DEBUG_PROJECT
	cout << "projector:" << endl;
	cout << "width: " << sl_parameter.projector_width << endl;
	cout << "height: " << sl_parameter.projector_height << endl;
	cout << "invert: " << sl_parameter.projector_invert << endl << endl;
#endif

	//Read gain parameters
	sl_parameter.camera_gain = slparameter_file["gain_parameters"]["camera_gain"];
	sl_parameter.projector_gain = slparameter_file["gain_parameters"]["projector_gain"];
#ifdef DEBUG_PROJECT
	cout << "gain:" << endl;
	cout << "camera_gain: " << sl_parameter.camera_gain << endl;
	cout << "projector_gain: " << sl_parameter.projector_gain << endl << endl;
#endif

	//Read camera calibration  parameters
	sl_parameter.camera_board_size.width = slparameter_file["camera_calibration_parameters"]["board_size_width"];
	sl_parameter.camera_board_size.height = slparameter_file["camera_calibration_parameters"]["board_size_height"];
	sl_parameter.camera_square_size.width = slparameter_file["camera_calibration_parameters"]["square_size_width"];
	sl_parameter.camera_square_size.height = slparameter_file["camera_calibration_parameters"]["square_size_height"];
	sl_parameter.frame_amount = slparameter_file["camera_calibration_parameters"]["frame_amount"];
#ifdef DEBUG_PROJECT
	cout << "camera calibration:" << endl;
	cout << "board_size_width: " << sl_parameter.camera_board_size.width << endl;
	cout << "board_size_height: " << sl_parameter.camera_board_size.height << endl;
	cout << "square_size_width: " << sl_parameter.camera_square_size.width << endl;
	cout << "square_size_height: " << sl_parameter.camera_square_size.height << endl;
	cout << "frame_amount: " << sl_parameter.frame_amount << endl << endl;
#endif

	//Read projector calibration  parameters
	sl_parameter.projector_board_size.width = slparameter_file["projector_calibration_parameters"]["board_size_width"];
	sl_parameter.projector_board_size.height = slparameter_file["projector_calibration_parameters"]["board_size_height"];
	sl_parameter.projector_square_size.width = slparameter_file["projector_calibration_parameters"]["square_size_width"];
	sl_parameter.projector_square_size.height = slparameter_file["projector_calibration_parameters"]["square_size_height"];
#ifdef DEBUG_PROJECT
	cout << "projector calibration:" << endl;
	cout << "board_size_width: " << sl_parameter.projector_board_size.width << endl;
	cout << "board_size_height: " << sl_parameter.projector_board_size.height << endl;
	cout << "square_size_width: " << sl_parameter.projector_square_size.width << endl;
	cout << "square_size_height: " << sl_parameter.projector_square_size.height << endl << endl;
#endif

	//Read scan options
	sl_parameter.colum_scan_flag = (int)slparameter_file["scan_colums_flag"];
	sl_parameter.row_scan_flag = (int)slparameter_file["scan_rows_flag"];
	sl_parameter.project_capture_delay = slparameter_file["project_capture_delay"];
	sl_parameter.contrast_threshold = slparameter_file["contrast_threshold"];
	sl_parameter.distance_range[0] = slparameter_file["distance_range_min"];
	sl_parameter.distance_range[1] = slparameter_file["distance_range_max"];
	sl_parameter.distance_reject = slparameter_file["distance_reject"];
	sl_parameter.background_depth_threshold = slparameter_file["background_depth_threshold"];
#ifdef DEBUG_PROJECT
	cout << "colum_scan_flag: " << sl_parameter.colum_scan_flag << endl;
	cout << "row_scan_flag: " << sl_parameter.row_scan_flag << endl;
	cout << "project_capture_delay: " << sl_parameter.project_capture_delay << endl;
	cout << "contrast_threshold: " << sl_parameter.contrast_threshold << endl;
	cout << "distance_range_min: " << sl_parameter.distance_range[0] << endl;
	cout << "distance_range_max: " << sl_parameter.distance_range[1] << endl;
	cout << "distance_reject: " << sl_parameter.distance_reject << endl;
	cout << "background_depth_threshold: " << sl_parameter.background_depth_threshold << endl<<endl;
#endif
	slparameter_file.release();	 //must release file

	//Open calibration file 
	FileStorage slcalibration_file(".\\output\\calibration\\projector\\slcalibration.xml", FileStorage::READ);
	if (!slcalibration_file.isOpened())
	{
		cerr << "Failed to open slcalibration.xml" << endl;
		return -1;
	}
	//read intrinsic and distortion parmeters
	slcalibration_file["CameraIntrinsicMatrix"] >> sl_calibration.camera_intrinsic;
	slcalibration_file["CameraDistortionCofficients"] >> sl_calibration.camera_distortion;
	slcalibration_file["ProjectorIntrinsicMatrix"] >> sl_calibration.projector_intrinsic;
	slcalibration_file["ProjectorDistortionCofficients"] >> sl_calibration.projector_distortion;
	slcalibration_file["CameraIntrinsicCalibrationFlag"] >> sl_calibration.camera_intrinsic_calibration_flag;
	slcalibration_file["ProjectorIntrinsicCalibrationFlag"]>> sl_calibration.projector_intrinsic_calibration_flag;
	slcalibration_file.release();

#ifdef DEBUG_PROJECT
	cout << "camera intrinsic matrix: " << endl << sl_calibration.camera_intrinsic << endl;
	cout << "camera distortion cofficients: " << endl << sl_calibration.camera_distortion << endl;
	cout << "projector intrinsic matrix: " << endl << sl_calibration.projector_intrinsic << endl;
	cout << "projector distortion cofficients: " << endl << sl_calibration.projector_distortion << endl;
	cout<<"camera_intrinsic_calibration_flag: "<< sl_calibration.camera_intrinsic_calibration_flag<<endl;
	cout<<"projector_intrinsic_calibration_flag: " << sl_calibration.projector_intrinsic_calibration_flag<<endl<<endl;
#endif

	//open extrinsic file
	FileStorage extrinsic_file(".\\output\\calibration\\extrinsic\\extrinsic.xml", FileStorage::READ);
	if (!extrinsic_file.isOpened())
	{
		cerr << "Failed to open extrinsic.xml!" << endl;
		return -1;
	}
	//read intrinsic and distortion parmeters
	extrinsic_file["CameraRotationVectors"]>>sl_calibration.camera_extrinsic_rotation;
	extrinsic_file["CameraTranslationVectors"] >> sl_calibration.camera_extrinsic_translation;
	extrinsic_file["ProjectorRotationVectors"] >>sl_calibration.projector_extrinsic_rotation ;
	extrinsic_file["ProjectorTranslationVectors"] >> sl_calibration.projector_extrinsic_translation;
	extrinsic_file["CameraProjectorExtrinsicCalibrationFlag"] >> sl_calibration.camera_projector_extrinsic_calibration_flag;

#ifdef DEBUG_PROJECT
	cout << "camera rotation vectors: " << endl << sl_calibration.camera_extrinsic_rotation << endl;
	cout << "camera translation vectors: " << endl << sl_calibration.camera_extrinsic_translation << endl;
	cout << "projector rotation vectors: " << endl << sl_calibration.projector_extrinsic_rotation << endl;
	cout << "projector translation vectors: " << endl << sl_calibration.projector_extrinsic_translation << endl;
	cout << "camera projector extrinsic calibration flag: " << sl_calibration.camera_projector_extrinsic_calibration_flag << endl;
#endif
	extrinsic_file.release();
	cout << "............................................................." << endl << endl;
	return 0;
}

