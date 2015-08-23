#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


#include "structuredlight.h"

#include "DLP4500_API.h"
#include "usb.h"
#include "projector.h"

//#define USE_LCD
#define USE_DLP4500


#ifdef USE_LCD
int ProjectorInitialize(SlParameter &sl_parameter)
{
	//show full white projector background pattern
	sl_parameter.projector_background_pattern = Mat::zeros(sl_parameter.projector_height, sl_parameter.projector_width, CV_8U);
	sl_parameter.projector_background_pattern.setTo(255);
	imshow("projector window", sl_parameter.projector_background_pattern);
	waitKey(1000);
	cout << "projector initialize successful......" << endl << endl;
	return 0;
}

#elif defined(USE_DLP4500)
int ProjectorInitialize(SlParameter &sl_parameter)
{
	bool temp = false;
	USB_Open();
	temp=USB_IsConnected();
	LCR_SetMode(true); //pattern mode
	LCR_PatternDisplay(0);
	LCR_PatternDisplay(2);
	return 0;
}

#endif