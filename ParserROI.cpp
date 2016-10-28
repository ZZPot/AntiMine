#define _CRT_RAND_S
#include "ParserROI.h"
#include <opencv2/tracking/tracker.hpp>
#include <stdlib.h>

#pragma warning(disable: 4996)

parser_roi::parser_roi()
{
	_first_call = true;
}
bool parser_roi::Parse(cv::Mat img, field_params* params)
{
	if(_first_call)
	{
		_first_call = false;
		std::string temp_name = CreateRandomName(5);
		_roi = cv::selectROI(temp_name, img);
		cv::destroyWindow(temp_name);
	}
	cv::Mat img_roi = img(_roi);
	return ParseROI(img_roi, params);
}
mine_cell parser_roi::ParseCell(cv::Mat img, unsigned row, unsigned col)
{
	if(_first_call)
	{
		_first_call = false;
		std::string temp_name = CreateRandomName(5);
		_roi = cv::selectROI(temp_name, img);
		cv::destroyWindow(temp_name);
	}
	cv::Mat img_roi = img(_roi);
	return ParseCellROI(img_roi, row, col);
}
std::string CreateRandomName(unsigned num_chars, std::string prefix, std::string postfix)
{
	std::string new_name = prefix;
	if(num_chars > RANDOM_NAME_MAX_CHARS)
		num_chars = RANDOM_NAME_MAX_CHARS;
	unsigned u_rand;
	char rand_chunk[RANDOM_CHUNK_SIZE + 1];
	for(; num_chars; num_chars -= cv::min<unsigned>(num_chars, RANDOM_CHUNK_SIZE))
	{
		rand_s(&u_rand);
		itoa(u_rand % (unsigned)pow(10, cv::min<unsigned>(num_chars, RANDOM_CHUNK_SIZE)), rand_chunk, 10);
		new_name += rand_chunk;
	}
	new_name += postfix;
	return new_name;
}