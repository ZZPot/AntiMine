#pragma once
#include "../ParserROI.h"
#include "../MinerDisplay.h"

/*
	parser_xp - parse winmine of WindowsXP.
*/

#define BORDER_THICKNESS	3
#define CELL_SIZE			16

extern std::vector<cv::Scalar> frame_colors_xp;
extern std::vector<cv::Mat> templates_xp;
extern std::vector<cv::Scalar> field_colors_xp;

class parser_xp: public parser_roi, public cells_display
{
public:
	parser_xp();	
	void Reset();
	void Display();
protected:
	cv::Rect GetFieldRect(cv::Mat img_roi);
	bool ParseROI(cv::Mat img_roi, field_params* params);
	mine_cell ParseCellROI(cv::Mat img_roi, unsigned row, unsigned col);
	void GetParams(cv::Mat field_roi);
protected:
	field_params _params;
	cv::Rect field_rect;
};

mine_cell GetCell(cv::Mat img);
bool HasTemplate(cv::Mat img, cv::Mat temp);