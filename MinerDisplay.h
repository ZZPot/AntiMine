#pragma once
#include "MineField.h"
#include <opencv2/core.hpp>

#define COLOR_KNOWN		0
#define COLOR_GRID		9
#define COLOR_UNKNOWN	10
#define COLOR_MINE		11
#define COLOR_BG		12
#define COLOR_TRANSPARENT	13

extern std::vector<cv::Scalar> field_colors;

#define ICON_FLAG		0
#define ICON_MINE		1

extern std::vector<cv::Mat> icons;

#define DEFAULT_SIZE	20
#define FONT_SIZE		16


class cells_display
{
public:
	cells_display();
	void SetName(std::string wnd_name);
	void SetCellSize(unsigned size);
	virtual void Display() = 0;
protected:
	std::string _wnd_name;
	unsigned _size;
};
class mine_field_cpu_display: public mine_field_cpu, public cells_display
{
public:
	void Display();
};
class miner_display: public miner, public cells_display
{
public:
	void Display();
};
void DrawCell(cv::Mat img, cv::Rect rect, mine_cell cell);
void OverlayImg(cv::Mat img1, cv::Mat img2, cv::Scalar transparent, cv::Rect place, float opacity = 1);