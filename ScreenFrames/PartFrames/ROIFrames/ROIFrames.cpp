#include "ROIFrames.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/tracking/tracker.hpp>
#include "../../../common/common.hpp"

roi_frames::roi_frames()
{
	roi_frames::reset();
}
void roi_frames::reset()
{
	_roi_selected = false;
}
cv::Mat roi_frames::nextFrame()
{
	cv::Mat res;
	cv::Mat img = part_frames::nextFrame();
	if(!_roi_selected)
	{
		std::string temp_name = CreateRandomName(5, SELECT_ROI_PREFIX);
		_roi = cv::selectROI(temp_name, img);
		cv::destroyWindow(temp_name);
		// how to process closing window? Docs doesn't say anything
		_roi_selected = true;
	}
	res = img(_roi);
	return res;
}
cv::Point roi_frames::GetOffset()
{
	cv::Point res(-1, -1);
	if(_roi_selected)
		res = _roi.tl();
	return res;
}