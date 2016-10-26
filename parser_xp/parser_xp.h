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
	bool ParseROI(cv::Mat img_roi, field_params* params);
	void Display();
protected:
	field_params _params;
};

mine_cell GetCell(cv::Mat img);
bool HasTemplate(cv::Mat img, cv::Mat temp);
cv::Mat RemoveChannel(cv::Mat mat);