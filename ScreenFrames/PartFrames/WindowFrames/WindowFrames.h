#pragma once
#include "../PartFrames.h"
#include <windows.h>

#define WND_NAME_SELECT_WND "Click at window"

class window_frames: public part_frames
{
public:	
	window_frames(bool client = false);
	void reset();
	cv::Mat nextFrame();
	cv::Point GetOffset();
protected:
	bool _client;
	bool _window_selected;
	HWND _hwnd;
};
cv::Mat hwnd2matFull(HWND hwnd);
HWND GetWindowFromPoint();
void MousePoint(int mouse_event, int x, int y, int flags, void* param);