#pragma once
#include "../parser_win.h"

/*
	parser_xp - parse winmine of WindowsXP.
*/

#define BORDER_THICKNESS_XP		3
#define CELL_SIZE_XP			16

extern std::vector<cv::Scalar> frame_colors_xp;
extern std::vector<cv::Mat> templates_xp;
extern std::vector<cv::Scalar> field_colors_xp;

class parser_xp: public parser_win
{
public:	
protected:
	virtual cv::Rect GetFieldRect(cv::Mat img_roi);
	virtual cv::Rect GetCellRect(unsigned row, unsigned col);
	virtual mine_cell GetCell(cv::Mat img);
	virtual void InitParams();
};