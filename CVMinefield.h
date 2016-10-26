#pragma once
#include "MineField.h"
#include "opencv2/core.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/video.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videostab.hpp"
#include "opencv2/opencv_modules.hpp"

struct field_params
{
	unsigned rows;
	unsigned cols;
	unsigned size;
	std::vector<mine_cell> mines;
	std::vector<cv::Point> cells; // top left corner
	cv::Point reset;
};

class mine_parser
{
public:
	virtual ~mine_parser(){}
	virtual bool Parse(cv::Mat img, field_params* params) = 0;
};

class cv_minefield: public mine_field
{
public:
	cv_minefield();
	unsigned GetRows();
	unsigned GetCols();
	mine_cell CheckCell(unsigned row, unsigned col, bool flag);
	void Reset();
	void SetParser(mine_parser* parser);
	void SetFrameSource(cv::Ptr<cv::videostab::IFrameSource> frames);
	bool RefreshState();
protected:
	void ClickMine(unsigned row, unsigned col, bool flag);
protected:
	mine_parser* _parser;
	field_params _params;
	cv::Ptr<cv::videostab::IFrameSource> _frames;
	bool _last_frame;
};

void ClickAtPoint(cv::Point p, bool rmb = false);