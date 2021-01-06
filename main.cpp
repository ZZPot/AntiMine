#include "MinerDisplay.h"
#include "ScreenFrames/PartFrames/ROIFrames/ROIFrames.h"
#include "parser_win/parser_xp/parser_xp.h"
#include "parser_win/parser_7/parser_7.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/tracking/tracking.hpp>
#include <windows.h>
#include <iostream>
using namespace cv;

int main()
{
	cv_minefield cvm;
	parser_xp pars;
	Ptr<roi_frames> roif = makePtr<roi_frames>();
	cvm.SetParser(&pars);
	cvm.SetFrameSource(roif);
	cvm.RefreshState();
	
	//field_params params;
	miner_display my_miner;
	my_miner.Init(&cvm);
	cvm.Reset(); // just activate game window, not reset
	std::vector<miner_move> moves = my_miner.PrepareMoves();
	while(moves.size())
	{
		int res = my_miner.Move();
		waitKey(100);
		if(res != -1)
		{
			cvm.Reset();
			my_miner.Init(&cvm);
		}
	//	pars.Display();
	//	my_miner.Display();
		moves = my_miner.PrepareMoves();
	}
#ifdef _DEBUG
			std::cout<< "Complete" << std::endl; 
#endif
	waitKey(0);
	return 0;
}