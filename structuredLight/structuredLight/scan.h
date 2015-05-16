#ifndef _SCAN_H
#define _SCAN_H


void ColorizeWinter(Mat &src, Mat &dst, Mat &mask);
void DepthMapConvertToGray(const Mat &src, Mat &dst,const Mat &mask);
int SaveX3DFile(char *filename, Mat &points, Mat &colors, Mat &mask);

int GenerateGrayCode(SlParameter &sl_parameter);
int CaptureLivingImage(SlParameter &sl_parameter);

int RunScanningObject(SlParameter &sl_parameter);

int DecodeGrayCode(SlParameter &sl_parameter);
int DisplayDecodeResult(SlParameter &sl_parameter);
int IntersectLineWithPlane3D(double *ql, double *qv, double *w, double *intersect_point, double &namate);

int RunStructuredLight(SlParameter &sl_parameter, SlCalibration &sl_calibration);
int ReconstructDepthMap(SlParameter &sl_parameter, SlCalibration &sl_calibration);



#endif
