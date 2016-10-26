#include "MinerDisplay.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#pragma warning(disable: 4996)


cells_display::cells_display()
{
	_wnd_name = "";
	_size = DEFAULT_SIZE;
}
void cells_display::SetName(std::string wnd_name)
{
	_wnd_name = wnd_name;
}
void cells_display::SetCellSize(unsigned size)
{
	_size = size;
}

void mine_field_cpu_display::Display()
{
	cv::Mat img = cv::Mat(_size * _rows, _size * _cols, CV_8UC3);
	cv::Rect rect(0, 0, _size, _size);
	for(unsigned i = 0; i < _rows*_cols; i++)
	{
		rect.x = (i % _cols)*_size;
		rect.y = (i / _cols)*_size;
		DrawCell(img, rect, mine_cell(_mines[i] ? CELL_MINE : CELL_UNKNOWN));
	}
	cv::imshow(_wnd_name + "MineField", img);
}
void miner_display::Display()
{
	cv::Mat img = cv::Mat(_size * _rows, _size * _cols, CV_8UC3);
	cv::Rect rect(0, 0, _size, _size);
	for(unsigned i = 0; i < _rows*_cols; i++)
	{
		rect.x = (i % _cols)*_size;
		rect.y = (i / _cols)*_size;
		DrawCell(img, rect, _field[i]);
	}
	cv::imshow(_wnd_name + "Miner", img);
}
void DrawCell(cv::Mat img, cv::Rect rect, mine_cell cell)
{
	cv::Scalar bg_color, text_color;
	int img_num = -1;
	switch(cell.state)
	{
	case CELL_FLAG:
		img_num = ICON_FLAG;
	case CELL_UNKNOWN:
		bg_color = field_colors[COLOR_UNKNOWN];
		break;
	case CELL_SAFE:
		bg_color = field_colors[COLOR_KNOWN];
		text_color = field_colors[cell.param];
		break;
	case CELL_MINE:
		bg_color = field_colors[COLOR_MINE];
		img_num = ICON_MINE;
	}
	cv::rectangle(img, rect, bg_color, CV_FILLED);
	cv::rectangle(img, rect, field_colors[COLOR_GRID], 1);
	if(cell.state == CELL_UNKNOWN)
		return;
	if(img_num == -1)
	{
		char temp[5] = "";
		sprintf(temp, "%d", cell.param);
		rect.y += FONT_SIZE - 1;
		rect.x += 3;
		cv::putText(img, temp, rect.tl(), 
				cv::FONT_HERSHEY_PLAIN, 1, text_color, 2);
	}
	else
		OverlayImg(img, icons[img_num], field_colors[COLOR_TRANSPARENT], rect);
}
void OverlayImg(cv::Mat img1, cv::Mat img2, cv::Scalar transparent, cv::Rect place, float opacity)
{            
	cv::Mat resized_img2;
	cv::resize(img2, resized_img2, cv::Size(place.width, place.height), 0, 0, cv::INTER_NEAREST);
	cv::Mat mask; 
	cv::inRange(resized_img2, transparent, transparent, mask);
	mask = 255 - mask;
	cv::Mat imgROI = img1(place);
	cv::add(resized_img2 * opacity, imgROI * (1.0 - opacity), resized_img2, mask);
	resized_img2.copyTo(imgROI, mask);
}
std::vector<cv::Scalar> field_colors = {cv::Scalar(192, 192, 192),

										cv::Scalar(255, 0, 0),
										cv::Scalar(0, 128, 0),
										cv::Scalar(0, 0, 255),
										cv::Scalar(128, 0, 0),
										cv::Scalar(0, 0, 128),
										cv::Scalar(128, 128, 0),
										cv::Scalar(0, 0, 0),
										cv::Scalar(128, 128, 128),

										cv::Scalar(50, 50, 50), // GRID
										cv::Scalar(220, 220, 220), // unknown
										cv::Scalar(0, 0, 255), // mine
										cv::Scalar(50, 50, 50),  // bg
										cv::Scalar(255, 0, 255)  // transparent
										};
std::vector<cv::Mat> icons = {cv::imread("icons/flag.bmp"), cv::imread("icons/mine.bmp")};