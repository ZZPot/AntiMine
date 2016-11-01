#include "parser_7.h"
#include "../../common/FeatureDetector.h"
#include <iostream>
#include <windows.h>

#pragma warning(disable: 4800)
bool parser_7::ParseROI(cv::Mat img_roi, field_params* params)
{
#ifndef PARSE_FULL
	if(!(_params.rows * _params.cols))
	{
#endif
		field_rect = GetFieldRect(img_roi);
		if(field_rect.width == 0)
			return false;
		InitParams();
		if(!(_params.rows * _params.cols))
		{
			Reset();
			return false;
		}
		cv::Point offset = _roi.tl(); // from the original image
		offset.x += field_rect.x;
		offset.y += field_rect.y;
		_params.cells.resize(_params.rows * _params.cols);
		_params.mines.resize(_params.rows * _params.cols);
		for(unsigned i = 0; i < _params.rows; i++)
		for(unsigned j = 0; j < _params.cols; j++)
		{
			cv::Point cell_p;
			unsigned cell_num = i * _params.cols + j;
			cell_p.x = j * _params.size + offset.x + _params.size/2; // center
			cell_p.y = i * _params.size + offset.y + _params.size/2; // center
			_params.cells[cell_num] = cell_p;
		}
		prev_field = cv::Mat::zeros(field_rect.height/2 * 2, field_rect.width/2 * 2, CV_8UC3);
		DrawCellPoints();
#ifndef PARSE_FULL
	}
#endif
	cv::Mat field_roi = img_roi(field_rect);
	field_roi = CutCol(CutRow(field_roi, field_roi.rows/2), field_roi.cols/2);
	std::vector<unsigned> changed_cells = GetChanged(field_roi);
	for(auto changed_cell: changed_cells)
	{
		cv::Rect cell_rect = GetCellRect(changed_cell / _params.cols, changed_cell % _params.cols);
		_params.mines[changed_cell] = GetCell(field_roi(cell_rect));
		if(_params.mines[changed_cell].state == CELL_ERROR)
		{
#ifdef _DEBUG
		std::cout<< "CELL_ERROR: "<< changed_cell / _params.cols << " " << changed_cell % _params.cols <<std::endl; 
#endif
			Reset();
			return false;
		}
	}
	*params = _params;
	return true;
}
mine_cell parser_7::ParseCellROI(cv::Mat img_roi, unsigned row, unsigned col)
{
#ifndef PARSE_FULL
	if(!(_params.rows * _params.cols))
	{
#endif
		field_rect = GetFieldRect(img_roi);
		if(field_rect.width == 0)
			return mine_cell(CELL_ERROR);
		InitParams();
		if(!(_params.rows * _params.cols))
		{
			Reset();
			return mine_cell(CELL_ERROR);
		}
		if(!(IN_RANGE(row, 0, _params.rows - 1) && IN_RANGE(col, 0, _params.cols - 1)))
			return mine_cell(CELL_ERROR);
		_params.mines.resize(_params.rows * _params.cols);
#ifndef PARSE_FULL
	}
#endif
	if(!(IN_RANGE(row, 0, _params.rows) && IN_RANGE(col, 0, _params.cols)))
		return mine_cell(CELL_ERROR);
	unsigned cell_num = row * _params.cols + col;
	if( _params.mines[cell_num].state != CELL_UNKNOWN) // we already know it
		return _params.mines[cell_num];
	cv::Rect cell_rect = GetCellRect(row, col);
	cv::Mat field_roi = img_roi(field_rect);
	field_roi = CutCol(CutRow(field_roi, field_roi.rows/2), field_roi.cols/2);
	_params.mines[cell_num] = GetCell(field_roi(cell_rect));
	if(_params.mines[cell_num].state == CELL_ERROR)
	{
#ifdef _DEBUG
		std::cout<< "CELL_ERROR: "<< row << " " << col <<std::endl; 
#endif
		cv::imshow("err", field_roi);
		cv::waitKey(0);
		Reset();
		return mine_cell(CELL_ERROR);
	}
	return _params.mines[cell_num];		
}
cv::Rect parser_7::GetFieldRect(cv::Mat img_roi)
{
	cv::Mat img_hsv;
	cv::cvtColor(img_roi, img_hsv, CV_BGR2HSV);
	cv::Mat img_border_ranged;
	cv::inRange(img_hsv, frame_colors_7[0], frame_colors_7[1], img_border_ranged);
	std::vector<type_condition> cond;
	std::vector<int> checks;
	type_condition biggest;
	biggest.rect[0].width = img_roi.cols / 2;
	biggest.rect[0].height = img_roi.rows / 2;
	biggest.rect[1].width = img_roi.cols;
	biggest.rect[1].height = img_roi.rows;
	cond.push_back(biggest);
	checks.push_back(FEATURE_CHECK_SIZE);
	std::vector<Obj2d> obj = FindObjects(img_border_ranged, cond, checks, cv::RETR_EXTERNAL);
	if(!obj.size())
		return cv::Rect();
	cv::Rect res = obj[0].rect;
	res.x += BORDER_THICKNESS_7;
	res.y += BORDER_THICKNESS_7;
	res.width -= BORDER_THICKNESS_7*2;
	res.height -= BORDER_THICKNESS_7*2;
	return res;
}
cv::Rect parser_7::GetCellRect(unsigned row, unsigned col)
{
	return cv::Rect(col * _params.size + 3, row * _params.size + 4, _params.size - 5, _params.size - 5);
}
mine_cell parser_7::GetCell(cv::Mat img)
{
	for(unsigned i = 0; i < field_colors_7.size(); i++)
	{
		cv::Mat img_ranged;
		cv::inRange(img, field_colors_7[i].first, field_colors_7[i].second, img_ranged);
		if(GetMatched(img_ranged, templates_7[i]))
		{
			if(i <= 7)
				return mine_cell(CELL_SAFE, i+1);
			if(i == 8)
				return mine_cell(CELL_FLAG);
			if(i == 9)
				return mine_cell(CELL_MINE);
		}
	}
	cv::Mat img_hsv;
	cv::cvtColor(img, img_hsv, CV_BGR2HSV);
	for(unsigned i = 0; i < field_colors_hsv_7.size(); i++)
	{
		cv::Mat hsv_ranged;
		cv::inRange(img_hsv, field_colors_hsv_7[i].first, field_colors_hsv_7[i].second, hsv_ranged);
		if(GetMatched(hsv_ranged, cv::Mat::ones(hsv_ranged.size(), CV_8UC1)*255))
		{
			switch(i)
			{
			case 0:
				return mine_cell(CELL_SAFE, 0);
			case 1:
				return mine_cell();
			}
		}
	}
	return mine_cell(CELL_ERROR);
}
void parser_7::InitParams()
{
	_params.size = CELL_SIZE_7;
	_params.cols = field_rect.width / CELL_SIZE_7;
	_params.rows = field_rect.height / CELL_SIZE_7;
	_params.reset.x = -VK_ESCAPE;
	_params.reset.y = 200;
}
bool GetMatched(cv::Mat img, cv::Mat temp)
{
	cv::Mat dif;
	cv::absdiff(img, temp, dif);
	unsigned dif_count = cv::countNonZero(dif);
	unsigned full = cv::countNonZero(temp);
	if(dif_count /(float)full * 100 < COMPARE_PERCENT_7)
		return true;
	return false;
}
cv::Mat CutCol(cv::Mat img, int col_num)
{
	cv::Mat res;
	cv::Mat left = img.colRange(0, col_num);
	cv::Mat right = img.colRange(col_num + 1, img.cols);
	cv::hconcat(left, right, res);
	return res;
}
cv::Mat CutRow(cv::Mat img, int row_num)
{
	cv::Mat res;
	cv::Mat top = img.rowRange(0, row_num);
	cv::Mat bottom = img.rowRange(row_num + 1, img.rows);
	cv::vconcat(top, bottom, res);
	return res;
}
std::vector<std::pair<cv::Scalar, cv::Scalar>> field_colors_7 = {
							{cv::Scalar(185, 76, 58),cv::Scalar(255, 83, 73)},	// 1
							{cv::Scalar(0, 100, 26),cv::Scalar(15, 112, 35)},	//2
							{cv::Scalar(0, 0, 166),cv::Scalar(30, 30, 176)},	//3
							{cv::Scalar(125, 0, 0),cv::Scalar(140, 30, 30)},	//4
							{cv::Scalar(0, 0, 115),cv::Scalar(40, 40, 130)},	//5
							{cv::Scalar(118, 118, 0),cv::Scalar(140, 140, 10)},	//6
							{cv::Scalar(0, 0, 166),cv::Scalar(30, 30, 176)},	//7
							{cv::Scalar(0, 0, 166),cv::Scalar(30, 30, 176)},	//8
							{cv::Scalar(0, 0, 190),cv::Scalar(80, 80, 255)},	// flag
							{cv::Scalar(85, 85, 120),cv::Scalar(130, 130, 200)}	// mine
							};
std::vector<cv::Mat> templates_7 = {cv::imread("parser_win/parser_7/res/1.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_win/parser_7/res/2.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_win/parser_7/res/3.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_win/parser_7/res/4.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_win/parser_7/res/5.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_win/parser_7/res/6.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_win/parser_7/res/7.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_win/parser_7/res/8.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_win/parser_7/res/flag.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_win/parser_7/res/mine.bmp", cv::IMREAD_GRAYSCALE)
								};
std::vector<cv::Scalar> frame_colors_7 = {cv::Scalar(200/2, 10*2.55, 20*2.55),
										cv::Scalar(240/2, 35*2.55, 56*2.55)};
std::vector<std::pair<cv::Scalar, cv::Scalar>> field_colors_hsv_7 = {
		{cv::Scalar(200/2, 5*2.55, 80*2.55),cv::Scalar(235/2, 25*2.55, 100*2.55)},	// 0
		{cv::Scalar(180/2, 25*2.55, 70*2.55),cv::Scalar(235/2, 90*2.55, 100*2.55)},	// unknown
		};