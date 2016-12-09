#pragma once
#include "../parser_win.h"

/*
	parser_7 - parse winmine of Windows7.
*/

#define BORDER_THICKNESS_7	1
#define CELL_SIZE_7			18
#define COMPARE_PERCENT_7	25

extern std::vector<cv::Scalar> frame_colors_7;
extern std::vector<cv::Mat> templates_7;
extern std::vector<std::pair<cv::Scalar, cv::Scalar>> field_colors_7;
extern std::vector<std::pair<cv::Scalar, cv::Scalar>> field_colors_hsv_7;

class parser_7: public parser_win
{
public:	
	bool Parse(cv::Mat img_roi, field_params* params);
	mine_cell ParseCell(cv::Mat img_roi, unsigned row, unsigned col);
protected:
	virtual cv::Rect GetFieldRect(cv::Mat img_roi);
	virtual cv::Rect GetCellRect(unsigned row, unsigned col);
	virtual mine_cell GetCell(cv::Mat img);
	virtual void InitParams();
};
bool GetMatched(cv::Mat img, cv::Mat temp);
cv::Mat CutCol(cv::Mat img, int col_num);
cv::Mat CutRow(cv::Mat img, int row_num);