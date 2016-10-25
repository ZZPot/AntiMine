#include "parser_xp.h"


bool parser_xp::ParseROI(cv::Mat img_roi, field_params* params)
{
	_params.size = 16; // it's always 16
	/*
		A lot of work to do
	*/
	*params = _params;
	return false;
}
void parser_xp::Display()
{
	cv::Mat img = cv::Mat(_params.size * _params.rows, _params.size * _params.cols, CV_8UC3);
	cv::Rect rect(0, 0, _params.size, _params.size);
	for(unsigned i = 0; i < _params.rows*_params.cols; i++)
	{
		rect.x = (i % _params.cols)*_params.size;
		rect.y = (i / _params.cols)*_params.size;
		DrawCell(img, rect, _params.mines[i]);
	}
	cv::imshow(_wnd_name + "Parser_xp", img);
}