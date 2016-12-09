#include "CVMinefield.h"
#include <windows.h>
#include <iostream>
#include "common/common.hpp"

cv_minefield::cv_minefield()
{
	_parser = nullptr;
	_last_frame = false;
}
unsigned cv_minefield::GetRows()
{
	return _params.rows;
}
unsigned cv_minefield::GetCols()
{
	return _params.cols;
}
void cv_minefield::Reset()
{
	Sleep(100);
	if(_params.reset.x >= 0)
		ClickAtPoint(_params.reset+_frames->GetOffset());
	else
		PressKey(-_params.reset.x, _params.reset.y);
	_parser->Reset();
	Sleep(100); // full field repaint takes time (if there is no windows cv::waitKey() won't work)
	RefreshState();
}
std::vector<mine_cell> cv_minefield::GetFieldView()
{
	return _params.mines;
}
mine_cell cv_minefield::CheckCell(unsigned row, unsigned col, bool flag)
{
	if(!(IN_RANGE(row, 0, _params.rows-1) && IN_RANGE(col, 0, _params.cols-1)))
		return mine_cell();
	if(_params.mines[row * _params.cols + col].state == CELL_UNKNOWN)
	{
		ClickMine(row, col, flag);
#ifdef PARSE_SINGLE
		_params.mines[row * _params.cols + col] = _parser->ParseCell(_frames->nextFrame(), row, col);
		return  _params.mines[row * _params.cols + col];
#else
		RefreshState();
		if(!_last_frame)
			return mine_cell(CELL_ERROR);
#endif
	}
	return _params.mines[row * _params.cols + col];
}
void cv_minefield::SetParser(mine_parser* parser)
{
	_parser = parser;
}
void cv_minefield::SetFrameSource(cv::Ptr<part_frames> frames)
{
	_frames = frames;
}
void cv_minefield::ClickMine(unsigned row, unsigned col, bool flag)
{
	if(!_last_frame)
		return;
	ClickAtPoint(_params.cells[row * _params.cols + col] + _frames->GetOffset(), flag);
	Sleep(0); // cells should be repainted
}
bool cv_minefield::RefreshState()
{
	if(_parser == nullptr)
		return false;
	cv::Mat new_frame;
	// get new frame from frame_source
	new_frame = _frames->nextFrame();
	_last_frame = _parser->Parse(new_frame, &_params);
	return _last_frame;
}
void ClickAtPoint(cv::Point p, bool rmb)
{
	INPUT mouse_input;
	mouse_input.type = INPUT_MOUSE;
	SetCursorPos(p.x, p.y);
	mouse_input.mi.dx = p.x;
	mouse_input.mi.dy = p.y;
    mouse_input.mi.mouseData = 0;
    mouse_input.mi.dwFlags = rmb ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_LEFTDOWN;
    mouse_input.mi.time = 0;
	SendInput(1, &mouse_input, sizeof(INPUT));
	mouse_input.mi.dwFlags = rmb ? MOUSEEVENTF_RIGHTUP : MOUSEEVENTF_LEFTUP;
	SendInput(1, &mouse_input, sizeof(INPUT));
}
void PressKey(int key, int time)
{
	INPUT keyboard_input;
	keyboard_input.type = INPUT_KEYBOARD;
	keyboard_input.ki.wVk = key;
    keyboard_input.ki.time = 0;
	keyboard_input.ki.dwFlags = 0;
	SendInput(1, &keyboard_input, sizeof(INPUT));
	Sleep(time);
	keyboard_input.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &keyboard_input, sizeof(INPUT));
}
