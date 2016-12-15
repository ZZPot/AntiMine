#include "WindowFrames.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "../../../common/common.hpp"

window_frames::window_frames(bool client):
_client(client)
{
	window_frames::reset();
}
void window_frames::reset()
{
	_window_selected = false;
}
cv::Mat window_frames::nextFrame()
{
	if(!_window_selected)
	{
		_hwnd = GetWindowFromPoint();
	}
	cv::Mat res;
	if(!IsWindow(_hwnd))
		return res;
	_window_selected = true;
	if(_client)
	{
		res = hwnd2mat(_hwnd);
	}
	else
	{
		res = hwnd2matFull(_hwnd);
	}
	return res;
}
cv::Point window_frames::GetOffset()
{
	cv::Point res(-1, -1);
	if(!IsWindow(_hwnd))
		return res;
	res = cv::Point(0, 0);
	if(_client)
	{
		ClientToScreen(_hwnd, (POINT*)&res);
	}
	else
	{
		if(!MapWindowPoints(_hwnd, GetDesktopWindow(), (POINT*)&res, 1))
			res = cv::Point(-1, -1);
	}
	return res;
}
cv::Mat hwnd2matFull(HWND hwnd)
{
    HDC hwindowDC,hwindowCompatibleDC;

    int height,width,srcheight,srcwidth;
    HBITMAP hbwindow;
    cv::Mat src;
    BITMAPINFOHEADER  bi;

    hwindowDC=GetWindowDC(hwnd);
    hwindowCompatibleDC=CreateCompatibleDC(hwindowDC);
    SetStretchBltMode(hwindowCompatibleDC,COLORONCOLOR);

    RECT windowsize;    // get the height and width of the screen
    GetClientRect(hwnd, &windowsize);

    srcheight = windowsize.bottom;
    srcwidth = windowsize.right;
    height = windowsize.bottom/1;  //change this to whatever size you want to resize to
    width = windowsize.right/1;

    src.create(height,width,CV_8UC4);

    // create a bitmap
    hbwindow = CreateCompatibleBitmap( hwindowDC, width, height);
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height;  //this is the line that makes it draw upside down or not
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    // use the previously created device context with the bitmap
    SelectObject(hwindowCompatibleDC, hbwindow);
    // copy from the window device context to the bitmap device context
    StretchBlt( hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 0, 0, srcwidth, srcheight, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !
    GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO *)&bi, DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow

    // avoid memory leak
    DeleteObject (hbwindow);
    DeleteDC(hwindowCompatibleDC);
    ReleaseDC(hwnd, hwindowDC);
    return RemoveChannel(src);
}
HWND GetWindowFromPoint()
{
	HWND hwndDesktop = GetDesktopWindow();
	cv::Mat dt = hwnd2mat(hwndDesktop);
	cv::imshow(WND_NAME_SELECT_WND, dt);
	cv::Point mouse_point(-1, -1);
	cv::setMouseCallback(WND_NAME_SELECT_WND, MousePoint, &mouse_point);
	while(1)
	{
		int key = cv::waitKey(0);
		if(key == 27 || key == -1)
			break;
	}
	if(mouse_point.x == -1 && mouse_point.y == -1)
		return NULL;
	POINT mp = {mouse_point.x, mouse_point.y};
	HWND res = WindowFromPoint(mp);
	return res;
}
void MousePoint(int mouse_event, int x, int y, int flags, void* param)
{
	cv::Point* mouse_point = (cv::Point*) param;
	if(mouse_event == cv::EVENT_LBUTTONUP)
	{
		mouse_point->x = x;
		mouse_point->y = y;
		cv::destroyWindow(WND_NAME_SELECT_WND);
	}
}