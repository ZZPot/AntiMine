#include "parser_win.h"
#include "FeatureDetector/FeatureDetector.h"
#include "FeatureDetector/Draw/DrawDebug.h"
#include "OpenCV_common.hpp"
#include <iostream>
#pragma warning(disable: 4800)
parser_win::parser_win()
{
	_params.cols = 0;
	_params.rows = 0;
}
bool parser_win::Parse(cv::Mat img_roi, field_params* params)
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
		_params.cells.resize(_params.rows * _params.cols);
		_params.mines.resize(_params.rows * _params.cols);
		for(unsigned i = 0; i < _params.rows; i++)
		for(unsigned j = 0; j < _params.cols; j++)
		{
			cv::Point cell_p;
			unsigned cell_num = i * _params.cols + j;
			cell_p = (cv::Point(j, i) * (unsigned)_params.size) + field_rect.tl() + _params.size/2;
			_params.cells[cell_num] = cell_p;
		}
		prev_field = cv::Mat::zeros(field_rect.height, field_rect.width, CV_8UC3);
		DrawCellPoints();
#ifndef PARSE_FULL
	}
#endif
	cv::Mat field_roi = img_roi(field_rect);
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
mine_cell parser_win::ParseCell(cv::Mat img_roi, unsigned row, unsigned col)
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
void parser_win::DrawCellPoints()
{
	if(!_params.cols)
		return;
	cell_points = cv::Mat::zeros(field_rect.height / _params.size * _params.size, field_rect.width / _params.size * _params.size, CV_8UC1);
	for(auto cell: _params.cells)
		cell_points.at<unsigned char>(cell.y - field_rect.y, cell.x - field_rect.x) = 255;
}
std::vector<unsigned> parser_win::GetChanged(cv::Mat new_field_img)
{
	std::vector<unsigned> res;
	if(!(_params.rows * _params.cols))
		return res;
	cv::Mat changed;
	cv::absdiff(prev_field, new_field_img, changed);
	cv::cvtColor(changed, changed, cv::COLOR_BGR2GRAY);
	changed *= 255; 
	std::vector<Obj2d> objects = FindObjects(changed, std::vector<type_condition>(), std::vector<int>(), cv::RETR_EXTERNAL);
	for(auto obj: objects)
	{
		DrawContours(obj.contours, {cv::Scalar::all(255)}, changed, cv::Point(), 1, cv::FILLED);
	}
	bitwise_and(changed, cell_points, changed);
	std::vector<cv::Point> center_points;
	cv::findNonZero(changed, center_points);
	res.reserve(center_points.size());
	for(auto point: center_points)
	{
		res.push_back((point.y /_params.size)  * _params.cols + (point.x /_params.size));
	}
	prev_field = new_field_img;
	return res;
}
void parser_win::Reset()
{
	_params.cols = 0;
	_params.mines.clear();
}
void parser_win::Display()
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
	cv::imshow(_wnd_name + " parser", img);
}
bool HasTemplate(cv::Mat img, cv::Mat temp)
{
	cv::Mat matched;
	cv::matchTemplate(img, temp, matched, cv::TM_SQDIFF);
	matched.convertTo(matched, CV_8UC1);
	matched = 255 - matched;
	return cv::countNonZero(matched);
}