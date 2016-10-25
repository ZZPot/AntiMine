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
	virtual unsigned GetRows();
	virtual unsigned GetCols();
	mine_cell CheckCell(unsigned row, unsigned col);

	void SetParser(mine_parser* parser);
	void SetFrameSource(cv::Ptr<cv::videostab::IFrameSource> frames);
protected:
	void SwapMine(unsigned row, unsigned col);
	bool RefreshState();
protected:
	mine_parser* _parser;
	field_params _params;
	cv::Ptr<cv::videostab::IFrameSource> _frames;
};