#include "MinerDisplay.h"
#include "DTFrames.h"
#include "parser_xp/parser_xp.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/tracking/tracker.hpp>
#include <windows.h>
using namespace cv;

int main()
{
	mine_field_cpu_display minefield;
	minefield.Init(16, 30, 99);
	minefield.Display();
	miner_display my_miner;
	my_miner.Init(&minefield);
	while(my_miner.PrepareMoves().size())
	{
		my_miner.Move();
		my_miner.Display();
		minefield.Display();
		if(waitKey(0) == 27)
		{
			minefield.Init(16, 30, 99);
			my_miner.Init(&minefield);
		}
	}
	waitKey(0);
	return 0;
}
/*
Рандом у сэйф
Не рандомить в 100% окружении мин
трюк 11
трюк 121
*/