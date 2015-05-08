#include <iostream>
#include <string>
#include <direct.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "structuredlight.h"
#include "auxiliaryfunctions.h"
#include "phaseshift.h"

using namespace std;
using namespace cv;

template<typename T>
T max(T a, T b, T c){ return a > b ? (a > c ? a : c) : (b > c ? b : c); }

template<typename T>
T min(T a, T b, T c){ return a < b ? (a < c ? a : c) : (b < c ? b : c); }


PhaseShift::PhaseShift(PhaseShiftMode m1, PhaseShiftScanMode m2) : phaseshift_mode(m1), phaseshift_scan_mode(m2)
{
	Init(); 
}

void PhaseShift::Init()
{
	_mkdir("./output");
	_mkdir("./output/phaseshift");
	switch (phaseshift_mode){
		case PHASESHIFT_THREE:
			_mkdir("./output/phaseshift/phaseshift_three");
			if (phaseshift_scan_mode==VERTICAL)
				_mkdir("./output/phaseshift/phaseshift_three/vertical");
			else
				_mkdir("./output/phaseshift/phaseshift_three/horizontal");
			break;
		case PHASESHIFT_RGB:
			_mkdir("./output/phaseshift/phaseshift_rgb"); 
			if (phaseshift_scan_mode == VERTICAL)
				_mkdir("./output/phaseshift/phaseshift_rgb/vertical");
			else
				_mkdir("./output/phaseshift/phaseshift_rgb/horizontal");
			break;
		default:
			break;
	}
	//Read structuredlight setting parameters from structuredlight.xml
	FileStorage configuration_file("slparameter.xml", FileStorage::READ);
	if (!configuration_file.isOpened())
	{
		cout << "Failed to open slparameter.xml!" << endl;
		return ;
	}
	//Read image parameters
	image_width = configuration_file["camera"]["width"];
	image_height = configuration_file["camera"]["height"];
#ifdef DEBUG_PROJECT
	cout << "image:" << endl;
	cout << "width: " << image_width  << endl;
	cout << "height: " << image_height << endl << endl;
#endif
	//Read pattern parameters
	pattern_width = configuration_file["projector"]["width"];
	pattern_height = configuration_file["projector"]["height"];
	pattern_period = configuration_file["projector"]["period"];
#ifdef DEBUG_PROJECT
	cout << "projector:" << endl;
	cout << "width: " << pattern_width << endl;
	cout << "height: " << pattern_height << endl;
	cout << "period: " << pattern_period << endl << endl;
#endif

	//Read gain parameters
	image_gain = configuration_file["gain_parameters"]["camera_gain"];
	pattern_gain = configuration_file["gain_parameters"]["projector_gain"];
#ifdef DEBUG_PROJECT
	cout << "gain:" << endl;
	cout << "image_gain: " << image_gain << endl;
	cout << "pattern_gain: " << pattern_gain << endl << endl;
#endif

	//Read scan options
	project_capture_delay = configuration_file["project_capture_delay"];
	int temp = configuration_file["three_noise_threshold"];
	three_noise_threshold=temp/100.0f;
#ifdef DEBUG_PROJECT
	cout << "project_capture_delay: " << project_capture_delay << endl;
	cout<<"three_noise_threshold:"<<three_noise_threshold<<endl;
#endif

	switch (phaseshift_mode){
	case PHASESHIFT_THREE:
		three_pattern_phase1 = Mat(pattern_height, pattern_width, CV_8UC1, Scalar(0));
		three_pattern_phase2 = Mat(pattern_height, pattern_width, CV_8UC1, Scalar(0));
		three_pattern_phase3 = Mat(pattern_height, pattern_width, CV_8UC1, Scalar(0));

		three_image1 = Mat(image_height, image_width, CV_8UC3, Scalar(0));
		three_image2 = Mat(image_height, image_width, CV_8UC3, Scalar(0));
		three_image3 = Mat(image_height, image_width, CV_8UC3, Scalar(0));
		three_image_gray1 = Mat(image_height, image_width, CV_8UC1, Scalar(0));
		three_image_gray2 = Mat(image_height, image_width, CV_8UC1, Scalar(0));
		three_image_gray3 = Mat(image_height, image_width, CV_8UC1, Scalar(0));
		break;

	case PHASESHIFT_RGB:
		rgb_pattern = Mat(pattern_height, pattern_width, CV_8UC3, Scalar(0));
		rgb_image = Mat(image_height, image_width, CV_8UC3, Scalar(0));
		break;
	default:
		break;
	}
	three_wrapped_atan = Mat(image_height, image_width, CV_8UC1, Scalar(0));
	three_wrapped_fast = Mat(image_height, image_width, CV_8UC1, Scalar(0));
	three_unwrapped_atan = Mat(image_height, image_width, CV_8UC1, Scalar(0));
	three_unwrapped_fast = Mat(image_height, image_width, CV_8UC1, Scalar(0));
	three_phase_mask = Mat(image_height, image_width, CV_8UC1, Scalar(0));
	three_color_image = Mat(image_height, image_width, CV_8UC3, Scalar(0));
}

int PhaseShift::GeneratePhaseShiftPattern()
{
	unsigned int row_index = 0, colum_index = 0;
	if (phaseshift_mode == PHASESHIFT_THREE){
		string save_directory("./output/phaseshift/phaseshift_three");
		string save_name("");

		if (phaseshift_scan_mode == VERTICAL){
			save_directory += "/vertical/pattern";
			_mkdir(save_directory.c_str());

			for (row_index = 0; row_index < pattern_height; ++row_index){
				if (row_index == 0){
					for (colum_index = 0; colum_index < pattern_width; ++colum_index){
						three_pattern_phase1.at<unsigned char>(0, colum_index) = 127.5* (cos(2 * M_PI*colum_index / pattern_period-2*M_PI/3)+1);
						three_pattern_phase2.at<unsigned char>(0, colum_index) = 127.5* (cos(2 * M_PI*colum_index / pattern_period) + 1);
						three_pattern_phase3.at<unsigned char>(0, colum_index) = 127.5* (cos(2 * M_PI*colum_index / pattern_period+2*M_PI/3) + 1);
					}
				}
				else{
					for (colum_index = 0; colum_index < pattern_width; ++colum_index){
						three_pattern_phase1.at<unsigned char>(row_index, colum_index) = three_pattern_phase1.at<unsigned char>(0, colum_index);
						three_pattern_phase2.at<unsigned char>(row_index, colum_index) = three_pattern_phase2.at<unsigned char>(0, colum_index);
						three_pattern_phase3.at<unsigned char>(row_index, colum_index) = three_pattern_phase3.at<unsigned char>(0, colum_index);
					}
				}
			}

			save_name = "";
			save_name = save_directory + "/v_three_phase1.jpg";
			imwrite(save_name, three_pattern_phase1);
			save_name = "";
			save_name = save_directory + "/v_three_phase2.jpg";
			imwrite(save_name, three_pattern_phase2);
			save_name = "";
			save_name = save_directory + "/v_three_phase3.jpg";
			imwrite(save_name, three_pattern_phase3);
		}

		else if (phaseshift_scan_mode == HORIZONTAL){
			save_directory += "/horizontal/pattern";
			_mkdir(save_directory.c_str());

			for (colum_index = 0; colum_index < pattern_width; ++colum_index){
				if (colum_index == 0){
					for (row_index = 0; row_index < pattern_height; ++row_index){
						three_pattern_phase1.at<unsigned char>(row_index, 0) = 127.5* (cos(2 * M_PI*row_index / pattern_period - 2 * M_PI / 3) + 1);
						three_pattern_phase2.at<unsigned char>(row_index, 0) = 127.5* (cos(2 * M_PI*row_index / pattern_period) + 1);
						three_pattern_phase3.at<unsigned char>(row_index, 0) = 127.5* (cos(2 * M_PI*row_index / pattern_period + 2 * M_PI / 3) + 1);
					}
				}
				else{
					for (row_index = 0; row_index < pattern_height; ++row_index){
						three_pattern_phase1.at<unsigned char>(row_index, colum_index) = three_pattern_phase1.at<unsigned char>(row_index, 0);
						three_pattern_phase2.at<unsigned char>(row_index, colum_index) = three_pattern_phase2.at<unsigned char>(row_index, 0);
						three_pattern_phase3.at<unsigned char>(row_index, colum_index) = three_pattern_phase3.at<unsigned char>(row_index, 0);
					}
				}
			}
			save_name = "";
			save_name = save_directory + "/h_three_phase1.jpg";
			imwrite(save_name, three_pattern_phase1);
			save_name = "";
			save_name = save_directory + "/h_three_phase2.jpg";
			imwrite(save_name, three_pattern_phase2);
			save_name = "";
			save_name = save_directory + "/h_three_phase3.jpg";
			imwrite(save_name, three_pattern_phase3);
		}
	}
	else if (phaseshift_mode == PHASESHIFT_RGB){
		string save_directory("./output/phaseshift/phaseshift_rgb");
		string save_name("");

		if (phaseshift_scan_mode == VERTICAL){
			save_directory += "/vertical/pattern";
			_mkdir(save_directory.c_str());
			for (row_index = 0; row_index < pattern_height; ++row_index){
				if (row_index == 0){
					for (colum_index = 0; colum_index < pattern_width; ++colum_index){
						rgb_pattern.at<Vec3b>(0, colum_index)[2] = 127.5* (cos(2 * M_PI*colum_index / pattern_period - 2 * M_PI / 3) + 1); //R
						rgb_pattern.at<Vec3b>(0, colum_index)[1] = 127.5* (cos(2 * M_PI*colum_index / pattern_period) + 1);				 //G
						rgb_pattern.at<Vec3b>(0, colum_index)[0] = 127.5* (cos(2 * M_PI*colum_index / pattern_period + 2 * M_PI / 3) + 1); //B
					}
				}
				else{
					for (colum_index = 0; colum_index < pattern_width; ++colum_index){
						rgb_pattern.at<Vec3b>(row_index, colum_index)[2] = rgb_pattern.at<Vec3b>(0, colum_index)[2];
						rgb_pattern.at<Vec3b>(row_index, colum_index)[1] = rgb_pattern.at<Vec3b>(0, colum_index)[1];
						rgb_pattern.at<Vec3b>(row_index, colum_index)[0] = rgb_pattern.at<Vec3b>(0, colum_index)[0];
					}
				}
			}

			save_name = "";
			save_name = save_directory + "/v_rgb.jpg";
			imwrite(save_name, rgb_pattern);
		}
		else if (phaseshift_scan_mode == HORIZONTAL){
			save_directory += "/horizontal/pattern";
			_mkdir(save_directory.c_str());
			for (colum_index = 0; colum_index < pattern_width; ++colum_index){
				if (colum_index == 0){
					for (row_index = 0; row_index < pattern_height; ++row_index){
						rgb_pattern.at<Vec3b>(row_index, 0)[2] = 127.5* (cos(2 * M_PI*row_index / pattern_period - 2 * M_PI / 3) + 1); //R
						rgb_pattern.at<Vec3b>(row_index, 0)[1] = 127.5* (cos(2 * M_PI*row_index / pattern_period) + 1);				 //G
						rgb_pattern.at<Vec3b>(row_index, 0)[0] = 127.5* (cos(2 * M_PI*row_index / pattern_period + 2 * M_PI / 3) + 1); //B
					}
				}
				else{
					for (row_index = 0; row_index < pattern_height; ++row_index){
						rgb_pattern.at<Vec3b>(row_index, colum_index)[2] = rgb_pattern.at<Vec3b>(row_index, 0)[2];
						rgb_pattern.at<Vec3b>(row_index, colum_index)[1] = rgb_pattern.at<Vec3b>(row_index, 0)[1];
						rgb_pattern.at<Vec3b>(row_index, colum_index)[0] = rgb_pattern.at<Vec3b>(row_index, 0)[0];
					}
				}
			}

			save_name = "";
			save_name = save_directory + "/h_rgb.jpg";
			imwrite(save_name, rgb_pattern);
		}
	}
	return 1;
}

int PhaseShift::ReadPhaseShiftPattern()
{
	if (phaseshift_mode== PHASESHIFT_THREE){
		string read_directory("./output/phaseshift/phaseshift_three");
		ostringstream read_name("");
		Mat pattern_temp(pattern_height, pattern_width, CV_8UC1, Scalar(0));

		if (phaseshift_scan_mode == VERTICAL){
			read_directory += "/vertical/pattern";
			for (int i = 1; i < 4; ++i){
				read_name.str("");
				read_name << read_directory<<"/v_three_phase" << i << ".jpg";
				pattern_temp = imread(read_name.str(), CV_LOAD_IMAGE_GRAYSCALE);
				if (pattern_temp.data==NULL){
					cout << "can not open " << read_name.str()<< endl;
					return 0;
				}
				else{
					switch (i){
						case 1:
							three_pattern_phase1 = pattern_temp; break;
						case 2:
							three_pattern_phase2 = pattern_temp; break;
						case 3:
							three_pattern_phase3 = pattern_temp; break;
						default:
							break;
					}
				}
			}
		}
		else if (phaseshift_scan_mode == HORIZONTAL){
			read_directory += "/horizontal/pattern";
			for (int i = 1; i < 4; ++i){
				read_name.str("");
				read_name << read_directory << "/h_three_phase" << i << ".jpg";
				pattern_temp = imread(read_name.str(), CV_LOAD_IMAGE_GRAYSCALE);
				if (pattern_temp.data == NULL){
					cout << "can not open " << read_name.str() << endl;
					return 0;
				}
				else{
					switch (i){
					case 1:
						three_pattern_phase1 = pattern_temp; break;
					case 2:
						three_pattern_phase2 = pattern_temp; break;
					case 3:
						three_pattern_phase3 = pattern_temp; break;
					default:
						break;
					}
				}	
			}
		}
	}
	else if (phaseshift_mode == PHASESHIFT_RGB){
		string read_directory("./output/phaseshift/phaseshift_rgb");
		ostringstream read_name("");
		Mat pattern_temp(pattern_height, pattern_width, CV_8UC3, Scalar(0));

		if (phaseshift_scan_mode == VERTICAL){
			read_directory += "/vertical/pattern";
			read_name.str("");
			read_name << read_directory << "/v_rgb.jpg";
			pattern_temp = imread(read_name.str(), CV_LOAD_IMAGE_COLOR);
			if (pattern_temp.data == NULL){
				cout << "can not open " << read_name.str()<<endl;
				return 0;
			}
			else{
				rgb_pattern = pattern_temp;
			}
		}
		else if (phaseshift_scan_mode == HORIZONTAL){
			read_directory += "/horizontal/pattern";
			read_name.str("");
			read_name << read_directory << "/h_rgb.jpg";
			pattern_temp = imread(read_name.str(), CV_LOAD_IMAGE_COLOR);
			if (pattern_temp.data == NULL){
				cout << "can not open " << read_name.str() << endl;
				return 0;
			}
			else{
				rgb_pattern = pattern_temp;
			}	
		}
	}
	return 1;
}

int PhaseShift::ScanObject(bool save_enable)
{
	Mat image_grab(image_height, image_width, CV_8UC3, Scalar(0));
	Mat project_pattern(pattern_height, pattern_width, CV_8UC1,Scalar(255));
	if (save_enable){
		//create a window to show capture result
		cout << "run phaseshift scan object....." << endl;
		namedWindow("scan object", WINDOW_AUTOSIZE);
		moveWindow("scan object", 50, 0);
		imshow("projector window", project_pattern*(pattern_gain / 100.0f));
		waitKey(100);
	}

	if (phaseshift_mode == PHASESHIFT_THREE){
		string save_directory("./output/phaseshift/phaseshift_three");
		string save_name("");
		if (phaseshift_scan_mode == VERTICAL){
			save_directory += "/vertical/image";
			_mkdir(save_directory.c_str());
			imshow("projector window", three_pattern_phase1*(pattern_gain / 100.0f));  //capture v_phase_1 image
			waitKey(project_capture_delay);
			if (GetImage(image_grab))
			{
				//waitKey(project_capture_delay);
				//GetImage(image_grab);
				three_image1 = image_grab*(image_gain / 100.0f);
				cvtColor(three_image1, three_image_gray1, COLOR_RGB2GRAY);
				imshow("scan object", three_image1);
				if (save_enable){
					save_name = "";
					save_name = save_directory + "/v_three_image1.jpg";
					imwrite(save_name.c_str(), three_image1);
					save_name = "";
					save_name = save_directory + "/v_three_image_gray1.jpg";
					imwrite(save_name.c_str(), three_image_gray1);
				}
			}

			imshow("projector window", three_pattern_phase2*(pattern_gain / 100.0f)); //capture v_phase_2 image
			waitKey(project_capture_delay);
			if (GetImage(image_grab))
			{
				//waitKey(project_capture_delay);
				//GetImage(image_grab);
				three_image2 = image_grab*(image_gain / 100.0f);
				cvtColor(three_image2, three_image_gray2, COLOR_RGB2GRAY);
				imshow("scan object", three_image2);
				if (save_enable){
					save_name = "";
					save_name = save_directory + "/v_three_image2.jpg";
					imwrite(save_name.c_str(), three_image2);
					save_name = "";
					save_name = save_directory + "/v_three_image_gray2.jpg";
					imwrite(save_name.c_str(), three_image_gray2);
				}
			}

			imshow("projector window", three_pattern_phase3*(pattern_gain / 100.0f)); //capture v_phase_3 image
			waitKey(project_capture_delay);
			if (GetImage(image_grab)){
				//waitKey(project_capture_delay);
				//GetImage(image_grab);
				three_image3 = image_grab*(image_gain / 100.0f);
				cvtColor(three_image3, three_image_gray3, COLOR_RGB2GRAY);
				imshow("scan object", three_image3);
				if (save_enable){
					save_name = "";
					save_name = save_directory + "/v_three_image3.jpg";
					imwrite(save_name.c_str(), three_image3);
					save_name = "";
					save_name = save_directory + "/v_three_image_gray3.jpg";
					imwrite(save_name.c_str(), three_image_gray3);
				}
			}
		}
		else if (phaseshift_scan_mode == HORIZONTAL){
			save_directory += "/horizontal/image";
			_mkdir(save_directory.c_str());
			imshow("projector window", three_pattern_phase1*(pattern_gain / 100.0f));  //capture h_phase_1 image
			waitKey(project_capture_delay);
			if (GetImage(image_grab))
			{
				//waitKey(project_capture_delay);
				//GetImage(image_grab);
				three_image1 = image_grab*(image_gain / 100.0f);
				cvtColor(three_image1, three_image_gray1, COLOR_RGB2GRAY);
				imshow("scan object", three_image1);
				if (save_enable){
					save_name = "";
					save_name = save_directory + "/h_three_image1.jpg";
					imwrite(save_name.c_str(), three_image1);
					save_name = "";
					save_name = save_directory + "/h_three_image_gray1.jpg";
					imwrite(save_name.c_str(), three_image_gray1);
				}
			}

			imshow("projector window", three_pattern_phase2*(pattern_gain / 100.0f)); //capture h_phase_2 image
			waitKey(project_capture_delay);
			if (GetImage(image_grab))
			{
				//waitKey(project_capture_delay);
				//GetImage(image_grab);
				three_image2 = image_grab*(image_gain / 100.0f);
				cvtColor(three_image2, three_image_gray2, COLOR_RGB2GRAY);
				imshow("scan object", three_image2);
				if (save_enable){
					save_name = "";
					save_name = save_directory + "/h_three_image2.jpg";
					imwrite(save_name.c_str(), three_image2);
					save_name = "";
					save_name = save_directory + "/h_three_image_gray2.jpg";
					imwrite(save_name.c_str(), three_image_gray2);
				}
			}

			imshow("projector window", three_pattern_phase3*(pattern_gain / 100.0f)); //capture h_phase_3 image
			waitKey(project_capture_delay);
			if (GetImage(image_grab)){
				//waitKey(project_capture_delay);
				//GetImage(image_grab);
				three_image3 = image_grab*(image_gain / 100.0f);
				cvtColor(three_image3, three_image_gray3, COLOR_RGB2GRAY);
				imshow("scan object", three_image3);
				if (save_enable){
					save_name = "";
					save_name = save_directory + "/h_three_image3.jpg";
					imwrite(save_name.c_str(), three_image3);
					save_name = "";
					save_name = save_directory + "/h_three_image_gray3.jpg";
					imwrite(save_name.c_str(), three_image_gray3);
				}
			}
		}
	}
	else if (phaseshift_mode == PHASESHIFT_RGB){
		string save_directory("./output/phaseshift/phaseshift_rgb");
		string save_name("");
		if (phaseshift_scan_mode == VERTICAL){
			save_directory += "/vertical/image";
			_mkdir(save_directory.c_str());
			imshow("projector window", rgb_pattern*(pattern_gain / 100.0f));  //capture v_rgb image
			waitKey(project_capture_delay);
			if (GetImage(image_grab))
			{
				//waitKey(project_capture_delay);
				//GetImage(image_grab);
				rgb_image = image_grab*(image_gain / 100.0f);
				imshow("scan object", rgb_image);
				if (save_enable){
					save_name = "";
					save_name = save_directory + "/v_rgb_image.jpg";
					imwrite(save_name.c_str(), rgb_image);
				}
			}
		}
		else if(phaseshift_scan_mode == HORIZONTAL){
			save_directory += "/horizontal/image";
			_mkdir(save_directory.c_str());
			imshow("projector window", rgb_pattern*(pattern_gain / 100.0f));  //capture h_rgb image
			waitKey(project_capture_delay);
			if (GetImage(image_grab))
			{
				//waitKey(project_capture_delay);
				//GetImage(image_grab);
				rgb_image = image_grab*(image_gain / 100.0f);
				imshow("scan object", rgb_image);
				if (save_enable){
					save_name = "";
					save_name = save_directory + "/h_rgb_image.jpg";
					imwrite(save_name.c_str(), rgb_image);
				}
			}
		}
	}
	if (save_enable){
		destroyWindow("scan object");
	}
	return 1;
}

int PhaseShift::ReadScanImage()
{
	if (phaseshift_mode == PHASESHIFT_THREE){
		string read_directory("./output/phaseshift/phaseshift_three");
		ostringstream read_name("");
		Mat image_temp(image_height, image_width, CV_8UC3, Scalar(0));
		Mat image_gray_temp(image_height, image_width, CV_8UC1, Scalar(0));

		if (phaseshift_scan_mode == VERTICAL){
			read_directory += "/vertical/image";
			for (int i = 1; i < 4; ++i){
				read_name.str("");
				read_name << read_directory << "/v_three_image" << i << ".jpg";
				image_temp = imread(read_name.str(), CV_LOAD_IMAGE_COLOR);
				read_name.str("");
				read_name << read_directory << "/v_three_image_gray" << i << ".jpg";
				image_gray_temp = imread(read_name.str(), CV_LOAD_IMAGE_GRAYSCALE);
				if ((image_temp.data == NULL)||(image_gray_temp.data==NULL)){
					cout << "can not open scan files"<<endl;
					return 0;
				}
				else{
					switch (i){
					case 1:
						three_image1 = image_temp;
						three_image_gray1 = image_gray_temp;
						break;
					case 2:
						three_image2 = image_temp;
						three_image_gray2 = image_gray_temp;
					case 3:
						three_image3 = image_temp;
						three_image_gray3 = image_gray_temp;
					default:
						break;
					}
				}
			}
		}
		else if (phaseshift_scan_mode == HORIZONTAL){
			read_directory += "/horizontal/image";
			for (int i = 1; i < 4; ++i){
				read_name.str("");
				read_name << read_directory << "/h_three_image" << i << ".jpg";
				image_temp = imread(read_name.str(), CV_LOAD_IMAGE_COLOR);
				read_name.str("");
				read_name << read_directory << "/h_three_image_gray" << i << ".jpg";
				image_gray_temp = imread(read_name.str(), CV_LOAD_IMAGE_GRAYSCALE);
				if ((image_temp.data == NULL) || (image_gray_temp.data == NULL)){
					cout << "can not open scan files" << endl;
					return 0;
				}
				else{
					switch (i){
					case 1:
						three_image1 = image_temp;
						three_image_gray1 = image_gray_temp;
						break;
					case 2:
						three_image2 = image_temp;
						three_image_gray2 = image_gray_temp;
					case 3:
						three_image3 = image_temp;
						three_image_gray3 = image_gray_temp;
					default:
						break;
					}
				}
			}
		}
	}
	else if (phaseshift_mode == PHASESHIFT_RGB){
		string read_directory("./output/phaseshift/phaseshift_rgb");
		ostringstream read_name("");
		Mat image_temp(image_height, image_width, CV_8UC3, Scalar(0));

		if (phaseshift_scan_mode == VERTICAL){
			read_directory += "/vertical/image";
			read_name << read_directory << "/v_rgb_image.jpg";
			image_temp = imread(read_name.str(), CV_LOAD_IMAGE_COLOR);
			if (image_temp.data == NULL){
				cout << "can not open " << read_name.str() << endl;
				return 0;
			}
			else{
				rgb_image = image_temp;
			}
		}
		else if (phaseshift_scan_mode == HORIZONTAL){
			read_directory += "/horizontal/image";
			read_name << read_directory << "/h_rgb_image.jpg";
			image_temp = imread(read_name.str(), CV_LOAD_IMAGE_COLOR);
			if (image_temp.data == NULL){
				cout << "can not open " << read_name.str() << endl;
				return 0;
			}
			else{
				rgb_image = image_temp;
			}
		}
	}
	return 1;
}

int PhaseShift::ComputeMaskRegion()
{
	int row_index = 0, colum_index = 0;
	if (phaseshift_mode == PHASESHIFT_THREE){
		unsigned phase1_value = 0, phase2_value = 0, phase3_value = 0;
		unsigned phase_min = 0, phase_max = 0, phase_range = 0, phase_sum = 0;
		double signal_noise_ratio = 0.0f;

		for (row_index = 0; row_index < image_height; ++row_index){
			for (colum_index = 0; colum_index < image_width; ++colum_index){
				phase1_value = three_image_gray1.at<unsigned char>(row_index, colum_index);
				phase2_value = three_image_gray2.at<unsigned char>(row_index, colum_index);
				phase3_value = three_image_gray3.at<unsigned char>(row_index, colum_index);
				phase_min = min(phase1_value, phase2_value, phase3_value);
				phase_max = max(phase1_value, phase2_value, phase3_value);
				phase_range = phase_max - phase_min;
				phase_sum = phase1_value + phase2_value + phase3_value;
				
				if (phase_sum != 0){
					signal_noise_ratio = (double)phase_range / phase_sum;
					if (signal_noise_ratio > three_noise_threshold){
						three_phase_mask.at<unsigned char>(row_index, colum_index) = 255;
						if (phase_max == phase1_value){
							for (int i = 0; i < 3;++i)
								three_color_image.at<Vec3b>(row_index, colum_index)[i] = three_image1.at<Vec3b>(row_index, colum_index)[i];
						}
						else if (phase_max == phase2_value){
							for (int i = 0; i < 3; ++i)
								three_color_image.at<Vec3b>(row_index, colum_index)[i] = three_image2.at<Vec3b>(row_index, colum_index)[i];
						}
						else{
							for (int i = 0; i < 3;++i)
								three_color_image.at<Vec3b>(row_index, colum_index)[i] = three_image3.at<Vec3b>(row_index, colum_index)[i];
						}
					}
				}
			}
		}
	}
	return 1;
}

int ThreePhaseDecodeAtan()
{
	return 1;
}
int ThreePhaseDecodeFast()
{
	return 1;
}