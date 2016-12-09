#pragma once
#include "../PartFrames.h"
#define SELECT_ROI_PREFIX	"Select region "

class roi_frames: public part_frames
{
public:	
	roi_frames();
	void reset();
	cv::Mat nextFrame();
	cv::Point GetOffset();
protected:
	bool _roi_selected;
	cv::Rect _roi;
};