#pragma once
#include "../ScreenFrames.h"
#include <opencv2/videostab/frame_source.hpp>

class part_frames: public screen_frames
{
public:	
	virtual cv::Point GetOffset() = 0;
};