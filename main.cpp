#include "MinerDisplay.h"
#include "DTFrames.h"
#include "parser_win/parser_xp/parser_xp.h"
#include "parser_win/parser_7/parser_7.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/tracking/tracker.hpp>
#include <windows.h>
using namespace cv;

int main()
{
	cv_minefield cvm;
	parser_7 pars;
	Ptr<dt_frames> dtf = makePtr<dt_frames>();
	cvm.SetParser(&pars);
	cvm.SetFrameSource(dtf);
	cvm.RefreshState();
	
	//field_params params;
	miner_display my_miner;
	my_miner.Init(&cvm);
	my_miner.Display();	
	pars.Display();
	cvm.Reset(); // just activate game window, not reset
	//waitKey(0);
	while(my_miner.PrepareMoves().size())
	{
		int res = my_miner.Move();
		pars.Display();
		my_miner.Display();
		waitKey(200);
		if(res != -1)
		{
			cvm.Reset();
			my_miner.Init(&cvm);
		}	
	}
#ifdef _DEBUG
			std::cout<< "Complete" << std::endl; 
#endif
	waitKey(0);
	return 0;
}