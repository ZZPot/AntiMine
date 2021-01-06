#include "parser_xp.h"
#include "FeatureDetector/FeatureDetector.h"
#include <iostream>
#pragma warning(disable: 4800)

cv::Rect parser_xp::GetFieldRect(cv::Mat img_roi)
{
	cv::Rect res;
	cv::Mat img_ranged[2];
	cv::inRange(img_roi, frame_colors_xp[0], frame_colors_xp[0], img_ranged[0]);
	cv::inRange(img_roi, frame_colors_xp[1], frame_colors_xp[1], img_ranged[1]);
	cv::Mat frames_ranged;	
	cv::bitwise_or(img_ranged[0], img_ranged[1], frames_ranged);
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
		return cv::Rect();
	res = obj[0].rect;
	res.x += BORDER_THICKNESS_XP;
	res.y += BORDER_THICKNESS_XP;
	res.width -= BORDER_THICKNESS_XP*2;
	res.height -= BORDER_THICKNESS_XP*2;
	return res;
}
cv::Rect parser_xp::GetCellRect(unsigned row, unsigned col)
{
	return cv::Rect(col * _params.size + 1, row * _params.size + 1, _params.size - 1, _params.size - 1);
}
mine_cell parser_xp::GetCell(cv::Mat img)
{
	//cv::imshow("Cell", img);
	//cv::waitKey(200);
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
	cv::Mat unknown_ranged;
	cv::inRange(img, field_colors_xp[0], field_colors_xp[0], unknown_ranged);
	if(HasTemplate(unknown_ranged, templates_xp[11]))
		return mine_cell(CELL_UNKNOWN);
	return mine_cell(CELL_ERROR);
}
void parser_xp::InitParams()
{
	_params.size = CELL_SIZE_XP;
	_params.cols = field_rect.width / CELL_SIZE_XP;
	_params.rows = field_rect.height / CELL_SIZE_XP;
	_params.reset.x = field_rect.x + field_rect.width/2;
	_params.reset.y = field_rect.y - 25;
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
std::vector<cv::Mat> templates_xp = {	cv::imread("parser_win/parser_xp/res/0.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_win/parser_xp/res/1.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_win/parser_xp/res/2.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_win/parser_xp/res/3.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_win/parser_xp/res/4.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_win/parser_xp/res/5.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_win/parser_xp/res/6.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_win/parser_xp/res/7.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_win/parser_xp/res/8.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_win/parser_xp/res/flag.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_win/parser_xp/res/mine.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_win/parser_xp/res/unknown.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_win/parser_xp/res/reset.bmp", cv::IMREAD_GRAYSCALE)
								};
std::vector<cv::Scalar> frame_colors_xp = {	cv::Scalar::all(128),
											cv::Scalar::all(255),
											cv::Scalar(0, 255, 255)};