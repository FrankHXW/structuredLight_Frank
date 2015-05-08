#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <time.h>

using namespace std;
using namespace cv;

#include "structuredlight.h"
#include "auxiliaryfunctions.h"
#include "calibration.h"
#include "scan.h"
#include "phaseshift.h"

clock_t clock_begin, clock_end;
int main()
{
	SlParameter slparameter;
	SlCalibration slcalibration;
	//Read structuredlight setting parameters from structuredlight.xml
	Read_slparameter(slparameter,slcalibration);

	//Create output directory
	CreateOutputDirectory(slparameter);

//	//Initialize projector
	namedWindow("projector window", 0);
	moveWindow("projector window", 1920, 0);
	setWindowProperty("projector window", WND_PROP_FULLSCREEN, 1);
	ProjectorInitialize(slparameter);

	//Initialize camera
	CameraInitialize(slparameter);
//
//	//Run camera calibration
////	RunCameraCalibration(slparameter,slcalibration);
//
//	//Run projector calibration
////	RunProjectorCalibration(slparameter,slcalibration);
//
//	//Run camera projector extrinsic calibration
////	RunCameraProjectorExtrinsicCalibration(slparameter, slcalibration);
//
//	EvaluteCameraProjectorGeometry(slparameter, slcalibration);
//	//Run structuredlight scan
//	RunStructuredLight(slparameter,slcalibration);

	PhaseShift slphaseshift(PHASESHIFT_THREE, VERTICAL);
//	slphaseshift.GeneratePhaseShiftPattern();
	slphaseshift.ReadPhaseShiftPattern();
	//while (1){
	//	clock_begin = clock();
	//	slphaseshift.ScanObject(false);
	//	clock_end = clock();
	//	cout << clock_end - clock_begin << endl;
	//}
	slphaseshift.ReadScanImage();
	slphaseshift.ComputeMaskRegion();

//	//Clear ram and camera
    Clear();
}






















