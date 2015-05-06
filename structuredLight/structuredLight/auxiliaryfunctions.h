//Describe: Header file for auxiliaryfunctions.h
//Author: Frank
//Date:   3/9/2015

#ifndef _AUXILIARYFUNCTIONS_H
#define _AUXILIARYFUNCTIONS_H

int Read_slparameter(SlParameter &sl_parameter, SlCalibration &sl_calibration);
int GetImage(Mat &frame_grab);
int CameraInitialize(SlParameter &sl_parameter);
int ProjectorInitialize(SlParameter &sl_parameter);
int GenerateChessBoardPattern(SlParameter &sl_parameter);
int CreateOutputDirectory(SlParameter &sl_parameter);
void Clear(void);


#endif




