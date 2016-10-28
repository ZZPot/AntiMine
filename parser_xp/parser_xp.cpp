#include "parser_xp.h"
#include "FeatureDetector.h"
#include <iostream>
#pragma warning(disable: 4800)
parser_xp::parser_xp()
{
	_params.cols = 0;
	_params.rows = 0;
}
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
	res.x += BORDER_THICKNESS;
	res.y += BORDER_THICKNESS;
	res.width -= BORDER_THICKNESS*2;
	res.height -= BORDER_THICKNESS*2;
	return res;
}
bool parser_xp::ParseROI(cv::Mat img_roi, field_params* params)
{
#ifndef PARSE_FULL
	if(!_params.cols)
	{
#endif
		field_rect = GetFieldRect(img_roi);
		if(field_rect.width == 0)
			return false;
		cv::Mat field_roi = img_roi(field_rect);
		GetParams(field_roi);
		if(_params.cols * _params.rows == 0)
			return false;
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
			cv::Rect cell_rect(j * _params.size + 1, i * _params.size + 1, _params.size - 1, _params.size - 1);
			_params.mines[cell_num] = GetCell(field_roi(cell_rect));
			if(_params.mines[cell_num].state == CELL_ERROR)
				return false;
		}
		_params.reset.x = field_rect.width/2 + offset.x;
		_params.reset.y = offset.y - 25;
#ifndef PARSE_FULL
	}
	else
	{
		cv::Mat field_roi = img_roi(field_rect);
		// check only unknown
		for(unsigned i = 0; i < _params.mines.size(); i++)
		{
			if( _params.mines[i].state != CELL_UNKNOWN) // we already know it
				continue;
			cv::Rect cell_rect((i % _params.cols) * _params.size + 1, (i / _params.cols) * _params.size + 1, _params.size - 1, _params.size - 1);
			_params.mines[i] = GetCell(field_roi(cell_rect));
			if(_params.mines[i].state == CELL_ERROR)
			{
#ifdef _DEBUG
			std::cout<< "CELL_ERROR: "<< i / _params.cols << " " << i % _params.cols <<std::endl; 
#endif
				Reset();
				return false;
			}
		}
	}
#endif
	*params = _params;
	return true;
}
mine_cell parser_xp::ParseCellROI(cv::Mat img_roi, unsigned row, unsigned col)
{
#ifndef PARSE_FULL
	if(!_params.cols)
	{
#endif
		field_rect = GetFieldRect(img_roi);
		if(field_rect.width == 0)
			return mine_cell(CELL_ERROR);
		cv::Mat field_roi = img_roi(field_rect);
		GetParams(field_roi);
		if(_params.cols * _params.rows == 0)
			return mine_cell(CELL_ERROR);
		if(!(IN_RANGE(row, 0, _params.rows - 1) && IN_RANGE(col, 0, _params.cols - 1)))
			return mine_cell(CELL_ERROR);
		cv::Point offset = _roi.tl(); // from the original image
		offset.x += field_rect.x;
		offset.y += field_rect.y;
		_params.cells.resize(_params.rows * _params.cols);
		_params.mines.resize(_params.rows * _params.cols);
		cv::Point cell_p;
		unsigned cell_num = row * _params.cols + col;
		cell_p.x = col * _params.size + offset.x + _params.size/2; // center
		cell_p.y = row * _params.size + offset.y + _params.size/2; // center
		_params.cells[cell_num] = cell_p;
		cv::Rect cell_rect(col * _params.size + 1, row * _params.size + 1, _params.size - 1, _params.size - 1);
		_params.mines[cell_num] = GetCell(field_roi(cell_rect));
		return _params.mines[cell_num];
#ifndef PARSE_FULL
	}
	else
	{
		if(!(IN_RANGE(row, 0, _params.rows) && IN_RANGE(col, 0, _params.cols)))
			return mine_cell(CELL_ERROR);
		cv::Mat field_roi = img_roi(field_rect);
		unsigned cell_num = row * _params.cols + col;
		if( _params.mines[cell_num].state != CELL_UNKNOWN) // we already know it
			return _params.mines[cell_num];
		cv::Rect cell_rect(col * _params.size + 1, row * _params.size + 1, _params.size - 1, _params.size - 1);
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
#endif
}
void parser_xp::GetParams(cv::Mat field_roi)
{
	_params.rows = field_rect.height / CELL_SIZE;
	_params.cols = field_rect.width / CELL_SIZE;
	_params.size = CELL_SIZE; // it's always 16
}
void parser_xp::Reset()
{
	_params.cols = 0; // yep, if cols == 0, that means that everything is fucked up and next time it parse all the image again.
	_params.mines.clear();
}
void parser_xp::Display()
{
	if(!(_params.rows * _params.cols))
		return;
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
bool HasTemplate(cv::Mat img, cv::Mat temp)
{
	cv::Mat matched;
	cv::matchTemplate(img, temp, matched, cv::TM_SQDIFF);
	matched.convertTo(matched, CV_8UC1);
	matched = 255 - matched;
	return cv::countNonZero(matched);
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
								cv::imread("parser_xp/res/mine.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_xp/res/unknown.bmp", cv::IMREAD_GRAYSCALE),
								cv::imread("parser_xp/res/reset.bmp", cv::IMREAD_GRAYSCALE)
								};
std::vector<cv::Scalar> frame_colors_xp = {	cv::Scalar::all(128),
											cv::Scalar::all(255),
											cv::Scalar(0, 255, 255)};