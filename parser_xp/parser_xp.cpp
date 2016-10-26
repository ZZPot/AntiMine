#include "parser_xp.h"
#include "FeatureDetector.h"
#include <iostream>

bool parser_xp::ParseROI(cv::Mat img_roi, field_params* params)
{
	img_roi = RemoveChannel(img_roi);
	// CHANNELS PROBLEM	
	cv::Mat img_ranged[2];
	cv::inRange(img_roi, frame_colors_xp[0], frame_colors_xp[0], img_ranged[0]);
	cv::inRange(img_roi, frame_colors_xp[1], frame_colors_xp[1], img_ranged[1]);
	cv::Mat frames_ranged;	
	cv::bitwise_or(img_ranged[0], img_ranged[1], frames_ranged);
	
	/*cv::imshow("roid", img_roi);
	cv::imshow("asd", img_ranged[0]);
	cv::imshow("asd1", img_ranged[1]);
	cv::waitKey(0);*/

	std::vector<type_condition> cond;
	std::vector<int> checks;
	type_condition biggest;
	biggest.rect[0].width = img_roi.cols / 2;
	biggest.rect[0].height = img_roi.rows / 2;
	biggest.rect[1].width = img_roi.cols;
	biggest.rect[1].height = img_roi.rows;
	cond.push_back(biggest);
	checks.push_back(FEATURE_CHECK_SIZE);
	std::vector<Obj2d> obj = FindObjects(frames_ranged, cond, checks, cv::RETR_EXTERNAL);
	if(!obj.size())
		return false;
	cv::Rect field_rect = obj[0].rect;
	field_rect.x += BORDER_THICKNESS;
	field_rect.y += BORDER_THICKNESS;
	field_rect.width -= BORDER_THICKNESS*2;
	field_rect.height -= BORDER_THICKNESS*2;
	cv::Mat field_roi = img_roi(field_rect);
	_params.rows = field_rect.height / CELL_SIZE;
	_params.cols = field_rect.width / CELL_SIZE;
	_params.size = CELL_SIZE; // it's always 16
	cv::Point shift = _roi.tl(); // from the original image
	shift.x += field_rect.x;
	shift.y += field_rect.y;
	_params.cells.resize(_params.rows * _params.cols);
	_params.mines.resize(_params.rows * _params.cols);
	for(unsigned i = 0; i < _params.rows; i++)
	for(unsigned j = 0; j < _params.cols; j++)
	{
		cv::Point cell_p;
		cell_p.x = j * _params.size + shift.x + _params.size/2; // center
		cell_p.y = i * _params.size + shift.y + _params.size/2; // center
		_params.cells[i * _params.cols + j] = cell_p;
		cv::Rect cell_rect(j * _params.size + 1, i * _params.size + 1, _params.size - 1, _params.size - 1);
		_params.mines[i * _params.cols + j] = GetCell(field_roi(cell_rect));
	}
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
mine_cell GetCell(cv::Mat img)
{
	int res = -1;
	for(unsigned i = 0; i < field_colors_xp.size(); i++)
	{
		cv::Mat img_ranged;
		cv::inRange(img, field_colors_xp[i], field_colors_xp[i], img_ranged);
		if(HasTemplate(img_ranged, templates_xp[i]))
			return mine_cell(CELL_SAFE, i);
	}
	cv::Mat img_ranged[2];
	cv::inRange(img, cv::Scalar(0, 0, 255), cv::Scalar(0, 0, 255), img_ranged[0]);
	cv::inRange(img, cv::Scalar(0, 0, 0), cv::Scalar(0, 0, 0), img_ranged[1]);
	cv::Mat img_flag_ranged;
	cv::bitwise_or(img_ranged[0], img_ranged[1], img_flag_ranged);
	if(HasTemplate(img_flag_ranged, templates_xp[9]))
		return mine_cell(CELL_FLAG);
	if(HasTemplate(img_ranged[1], templates_xp[10]))
		return mine_cell(CELL_MINE);
	return mine_cell();
}
bool HasTemplate(cv::Mat img, cv::Mat temp)
{
	cv::Mat matched;
	cv::matchTemplate(img, temp, matched, cv::TM_SQDIFF);
	matched.convertTo(matched, CV_8UC1);
	matched = 255 - matched;
	return cv::countNonZero(matched);
}
cv::Mat RemoveChannel(cv::Mat mat)
{
	std::vector<cv::Mat> channels;
	cv::split(mat, channels);
	channels.resize(3);
	cv::Mat img;
	cv::merge(channels, img);
	return img;
}
std::vector<cv::Scalar> field_colors_xp = {	cv::Scalar(192, 192, 192),
											cv::Scalar(255, 0, 0),	//1
											cv::Scalar(0, 128, 0),	//2
											cv::Scalar(0, 0, 255),	//3
											cv::Scalar(128, 0, 0),	//4
											cv::Scalar(0, 0, 128),	//5
											cv::Scalar(128, 128, 0),//6
											cv::Scalar(0, 0, 0),	//7
											cv::Scalar(128, 128, 128),//8
											};
std::vector<cv::Mat> templates_xp = {	cv::imread("parser_xp/res/0.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_xp/res/1.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_xp/res/2.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_xp/res/3.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_xp/res/4.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_xp/res/5.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_xp/res/6.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_xp/res/7.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_xp/res/8.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_xp/res/flag.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_xp/res/mine.bmp", cv::IMREAD_GRAYSCALE)
								};
std::vector<cv::Scalar> frame_colors_xp = {	cv::Scalar::all(128),
											cv::Scalar::all(255)};