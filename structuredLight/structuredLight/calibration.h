

int RunCameraCalibration(SlParameter &sl_parameter, SlCalibration &sl_calibration);
int RunProjectorCalibration(SlParameter &sl_parameter, SlCalibration &sl_calibration);
int RunCameraProjectorExtrinsicCalibration(SlParameter &sl_parameter, SlCalibration &sl_calibration);

int EvaluteCameraProjectorGeometry(SlParameter &sl_parameter, SlCalibration &sl_calibration);
int LineFitPlane(Mat &points, double *plane);


