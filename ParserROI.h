#pragma once
#include "CVMinefield.h"

/*
	Parser based on specified ROI
	First time it get screenshot and ask to select ROI.
	Please, do not move winmine window, ok?
*/
class parser_roi: public mine_parser
{
public:
	parser_roi();
	bool Parse(cv::Mat img, field_params* params);

	virtual bool ParseROI(cv::Mat img_roi, field_params* params) = 0;
protected:
	bool  _first_call;
	cv::Rect _roi;
};

#define RANDOM_NAME_MAX_CHARS 30
#define RANDOM_CHUNK_SIZE	5

std::string CreateRandomName(unsigned num_chars, std::string prefix = "", std::string postfix = "");