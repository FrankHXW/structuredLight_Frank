#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <windows.h>

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

unsigned char redLedCurrent = 50;
unsigned char greenLedCurrent = 65;
unsigned char bludeLedCurrent =60;

int ProjectorInitialize(SlParameter &sl_parameter)
{
	bool result = false;
	USB_Init();
	USB_Open();
	cout << "DLP4500 initialize ";
	if (USB_IsConnected()){
		LCR_SoftwareReset();
		Sleep(10000);		 //delay 10s to wait initializing
		while (USB_Open());	 //	must open usb after device reset
		if (LCR_GetMode(&result) == 0){		// checks the programmed pattern display modes
			if (result) cout << "pattern mode ";
			else cout << "video mode ";
		}
		cout << "success!"<<endl;
		if (LCR_PatternDisplay(0) >= 0){			//stop pattern display sequence
			LCR_SetLedEnables(true,true,true,true);
			for (unsigned int i = 1000000; i > 0; i--);
			LCR_SetLEDPWMInvert(true);
			LCR_SetLedCurrents(redLedCurrent, greenLedCurrent, bludeLedCurrent);
			for (unsigned int i = 1000000; i > 0; i--);
			LCR_SetPatternDisplayMode(false);		//pattern is fetched from splash memory
			for (unsigned int i = 1000000; i > 0; i--);
			unsigned int sequenceStatus = 0;
			LCR_ValidatePatLutData(&sequenceStatus);	//checks the programmed pattern display modes
			return 0;
		}
	}
	else{
		cout << "failed!" << endl;
		return 0;
	}
}

#endif