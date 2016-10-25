#pragma once
#include "../ParserROI.h"
#include "../MinerDisplay.h"

/*
	parser_xp - parse winmine of WindowsXP.
*/

class parser_xp: public parser_roi, public cells_display
{
public:
	bool ParseROI(cv::Mat img_roi, field_params* params);
	void Display();
protected:
	field_params _params;
};