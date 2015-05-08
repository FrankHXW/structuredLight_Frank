#ifndef _PHASESHIFT_H
#define _PHASESHIFT_H

using namespace std;
using namespace cv;

enum PhaseShiftMode{
	PHASESHIFT_THREE = 0,
	PHASESHIFT_RGB
};

enum PhaseShiftScanMode{
	VERTICAL=0,
	HORIZONTAL
};


class PhaseShift
{
public:
	PhaseShift()= default;
	PhaseShift(PhaseShiftMode m1, PhaseShiftScanMode m2);
	//image parameters
	int image_width=1024;	//capture image width 	
	int image_height=768;	//capture iamge height

	//pattern parameters
	int pattern_width=1024;	 //phaseshift pattern width
	int pattern_height=768;	 //phaseshift pattern height
	int pattern_period=64;   //phaseshift pattern period

	//gain parameters
	int image_gain=100;		//0-0.00,100-1.00,200-2.00
	int pattern_gain=100;   //0-0.00,100-1.00,200-2.00

	//scan and compute parameters
	int project_capture_delay=200;	 //frame delay between projection and image capture (in ms)
	float three_noise_threshold = 0.10;	 //three phaseshift signal noise ratio threshold,0.00~1.00

	//projected pattern 
	Mat three_pattern_phase1;
	Mat three_pattern_phase2;
	Mat three_pattern_phase3;
	Mat rgb_pattern;

	//captured image 
	Mat three_image1,three_image_gray1;
	Mat three_image2,three_image_gray2;
	Mat three_image3,three_image_gray3;
	Mat rgb_image;

	//decoded phase
	Mat three_wrapped_atan;
	Mat three_wrapped_fast;
	
	//unwrapped phase
	Mat three_unwrapped_atan;
	Mat three_unwrapped_fast;

	//image mask region 
	Mat three_phase_mask;
	Mat three_color_image;

	int GeneratePhaseShiftPattern();
	int ReadPhaseShiftPattern();
	int ScanObject(bool save_enable=true);
	int ReadScanImage();

	int ComputeMaskRegion();

	int ThreePhaseDecodeAtan();
	int ThreePhaseDecodeFast();
	
	int PhaseUnwrap();

private:
	PhaseShiftMode phaseshift_mode=PHASESHIFT_THREE;
	PhaseShiftScanMode phaseshift_scan_mode=VERTICAL;
	void Init();
};



#endif