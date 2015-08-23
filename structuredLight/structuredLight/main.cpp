#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <time.h>
#include <queue>

using namespace std;
using namespace cv;

#include "structuredlight.h"
#include "camera.h"
#include "projector.h"
#include "calibration.h"
#include "scan.h"
#include "phaseshift.h"

clock_t clock_begin, clock_end;

int main()
{
	SlParameter slparameter(1);
	SlCalibration slcalibration(1);

//	//Initialize projector
	//namedWindow("projector window", 0);
	//moveWindow("projector window", 1920, 0);
	//setWindowProperty("projector window", WND_PROP_FULLSCREEN, 1);
	ProjectorInitialize(slparameter);
//

//	//Initialize camera
	CameraInitialize(slparameter);
	Mat tmp(slparameter.camera_height, slparameter.camera_width, CV_8UC3, Scalar(0));
	int count = 0;
	while (1){
		clock_begin = clock();
		GetImage(tmp);
		imshow("camera", tmp);
		waitKey(1);
		cout << clock() - clock_begin<<" :"<<count++<<endl;
		count %= 22;
	}
////
//	//Run camera calibration
////	RunCameraCalibration(slparameter,slcalibration);
//
//	//Run projector calibration
////	RunProjectorCalibration(slparameter,slcalibration);
//
//	//Run camera projector extrinsic calibration
////	RunCameraProjectorExtrinsicCalibration(slparameter, slcalibration);
//
	EvaluteCameraProjectorGeometry(slparameter, slcalibration);
//	//Run structuredlight scan
	RunStructuredLight(slparameter,slcalibration);

	Mat temp;
	DepthMapConvertToGray(slparameter.depth_points, temp, slparameter.depth_valid, slparameter.distance_range[0], slparameter.distance_range[1]);


	PhaseShift slphaseshift(PHASESHIFT_THREE, VERTICAL);
	slphaseshift.GeneratePhaseShiftPattern();
	slphaseshift.ReadPhaseShiftPattern();
	//while (1){
	//	clock_begin = clock();
	//	slphaseshift.ScanObject(true);
	//	clock_end = clock();
	//	cout << clock_end - clock_begin << endl;
	//}
	slphaseshift.ReadScanImage();
	clock_begin = clock();
	slphaseshift.ComputeMaskRegion();
	cout << clock() - clock_begin<<endl;

	clock_begin = clock();
	slphaseshift.ThreePhaseDecodeAtan();
	cout << clock() - clock_begin<<endl;

	clock_begin = clock();
	slphaseshift.ThreePhaseComputeQuality();
	cout << clock() - clock_begin << endl;

	clock_begin = clock();
//	slphaseshift.ThreePhaseUnwrapBasedQuality();
	cout << clock() - clock_begin << endl;

	clock_begin = clock();
	slphaseshift.three_unwrap_process.setTo(0);
	slphaseshift.ThreePhaseUnwrapBasedMultiLevelQuality(slphaseshift.three_quality_average-slphaseshift.three_quality_deviation);
	cout << clock() - clock_begin << endl;

	clock_begin = clock();
	slphaseshift.ThreePhaseUnwrapBasedMultiLevelQuality(slphaseshift.three_quality_average - 2*slphaseshift.three_quality_deviation);
	cout << clock() - clock_begin << endl;

	clock_begin = clock();
	slphaseshift.MultiLevelQualitySolveRemain(slphaseshift.three_quality_average - 15 * slphaseshift.three_quality_deviation);
	cout << clock() - clock_begin << endl;

	clock_begin = clock();
	slphaseshift.DisplayUnwrapResult();
	cout << clock() - clock_begin << endl;

//	just test phaseshift's result ,use gray code's 3d reconstruct,need to modify 
	clock_begin = clock();
	slparameter.gray_valid_image = slphaseshift.three_unwrap_process;
	slparameter.decode_colum_scan_image=slphaseshift.three_unwrapped_result;
	slparameter.scene_color = slphaseshift.three_phase_color.clone();
	ReconstructDepthMap(slparameter,slcalibration);
	cout << clock() - clock_begin << endl;

	Mat temp1;
	DepthMapConvertToGray(slparameter.depth_points, temp1, slphaseshift.three_unwrap_process, slparameter.distance_range[0], slparameter.distance_range[1]);

	clock_begin = clock();
	char save_name[] = ".\\output\\phaseshift\\phaseshift_three\\vertical\\image\\pointCloud.x3d";
	SaveX3DFile(save_name, slparameter.depth_points, slparameter.depth_colors, slparameter.depth_valid);
	cout << clock() - clock_begin << endl;

	//Clear ram and camera
    CameraClear();
}






















