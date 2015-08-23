//Describe: Header file for auxiliaryfunctions.h
//Author: Frank
//Date:   3/9/2015

#ifndef CAMERA_H
#define CAMERA_H

using namespace std;
using namespace cv;

int GetImage(Mat &frame_grab);
int CameraInitialize(SlParameter &sl_parameter);

void CameraClear(void);


#endif




