#include "CVMinefield.h"


cv_minefield::cv_minefield()
{
	_parser = nullptr;
}
unsigned cv_minefield::GetRows()
{
	return _params.rows;
}
unsigned cv_minefield::GetCols()
{
	return _params.cols;
}
mine_cell cv_minefield::CheckCell(unsigned row, unsigned col)
{
	if((IN_RANGE(row, 0, _params.rows-1) && IN_RANGE(col, 0, _params.cols-1)) ||
		(row * _params.cols + col+1 > _params.mines.size()))
		return mine_cell();
	if(_params.mines[row * _params.cols + col].state == CELL_UNKNOWN)
	{
		SwapMine(row, col);
		if(!RefreshState())
			return mine_cell();
	}
	return _params.mines[row * _params.cols + col];
}
void cv_minefield::SetParser(mine_parser* parser)
{
	_parser = parser;
}
void cv_minefield::SetFrameSource(cv::Ptr<cv::videostab::IFrameSource> frames)
{

}
void cv_minefield::SwapMine(unsigned row, unsigned col)
{
	// click at _params.cells[row * _params.cols + col]
}
bool cv_minefield::RefreshState()
{
	if(_parser == nullptr)
		return false;
	cv::Mat new_frame;
	// get new frame from frame_source
	new_frame = _frames->nextFrame();
	return _parser->Parse(new_frame, &_params);
}