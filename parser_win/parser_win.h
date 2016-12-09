#pragma once
#include "../CVMinefield.h"
#include "../MinerDisplay.h"

/*
	parser_win - parse winmine. Common technique.
*/

class parser_win: public mine_parser, public cells_display
{
public:
	parser_win();	
	void Reset();
	void Display();
	bool Parse(cv::Mat img_roi, field_params* params);
	mine_cell ParseCell(cv::Mat img_roi, unsigned row, unsigned col);
protected:
	virtual cv::Rect GetFieldRect(cv::Mat img_roi) = 0;
	virtual cv::Rect GetCellRect(unsigned row, unsigned col) = 0;
	virtual mine_cell GetCell(cv::Mat img) = 0;
	virtual void InitParams() = 0;
	void DrawCellPoints();
	std::vector<unsigned> GetChanged(cv::Mat new_field_img); 
protected:
	field_params _params;
	cv::Rect field_rect;
	cv::Mat cell_points;
	cv::Mat prev_field;
};

bool HasTemplate(cv::Mat img, cv::Mat temp);
