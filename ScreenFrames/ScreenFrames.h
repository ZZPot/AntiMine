#pragma once
#include <windows.h>
#include <opencv2/videostab/frame_source.hpp>

class screen_frames: public cv::videostab::IFrameSource
{
public:	
	virtual void reset();
	virtual cv::Mat nextFrame();
};
cv::Mat hwnd2mat(HWND hwnd);
cv::Mat RemoveChannel(cv::Mat mat);