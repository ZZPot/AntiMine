#define _CRT_RAND_S
#include <stdlib.h> // for rand_s()
#include "MineField.h"
#include <iostream>
#pragma warning(disable: 4267 4018)

#undef USE_PATTERNS

bool CheckPatternValid(const field_pattern& pattern)
{
	if(	!pattern.field_part.size() || 
		!pattern.cols || 
		(pattern.field_part.size() % pattern.cols))
		return false;
	if(pattern.field_part.size() != pattern.checks.size())
		return false;
	return true;
}
field_pattern Rotate90(field_pattern pattern)
{
	field_pattern res;
	res.cols = pattern.field_part.size() / pattern.cols;
	res.field_part.resize(pattern.field_part.size());
	res.checks.resize(pattern.checks.size());
	for(auto move: pattern.moves)
	{
		unsigned old_col = move.col;
		move.col = (res.cols - 1) - move.row;
		move.row = old_col;
		res.moves.push_back(move);
	}
	for(unsigned i = 0; i < pattern.field_part.size(); i++)
	{
		unsigned old_row = i / pattern.cols;
		unsigned old_col = i % pattern.cols;
		unsigned new_row = old_col;
		unsigned new_col = (res.cols - 1) - old_row;
		unsigned new_cell_num = new_row * res.cols + new_col;
		res.field_part[new_cell_num] = pattern.field_part[i];
		res.checks[new_cell_num] = pattern.checks[i];
	}
	return res;
}
field_pattern MirrorHor(field_pattern pattern)
{
	unsigned rows = pattern.field_part.size() / pattern.cols;
	for(unsigned i = 0; i < rows; i++)
	for(unsigned j = 0; j < pattern.cols/2; j++)
	{
		unsigned cell_num_1 = i * pattern.cols + j;
		unsigned cell_num_2 = i * pattern.cols + (pattern.cols - j - 1);
		std::vector<mine_cell> temp = pattern.field_part[cell_num_1];
		pattern.field_part[cell_num_1] = pattern.field_part[cell_num_2];
		pattern.field_part[cell_num_2] = temp;
		int temp_check = pattern.checks[cell_num_1];
		pattern.checks[cell_num_1] = pattern.checks[cell_num_2];
		pattern.checks[cell_num_2] = temp_check;		
	}
	return pattern;
}
mine_cell::mine_cell(cell_state init_state, int init_param)
{
	state = init_state;
	param = init_param;
}
miner_move::miner_move(miner_action act, unsigned r, unsigned c)
{
	action = act;
	row = r;
	col = c;
}
mine_field_cpu::mine_field_cpu()
{
	_mines = nullptr;
	_rows = 0;
	_cols = 0;
}
mine_field_cpu::~mine_field_cpu()
{
	if(_mines != nullptr)
		delete [] _mines;
}
unsigned mine_field_cpu::GetRows()
{
	return _rows;
}
unsigned mine_field_cpu::GetCols()
{
	return _cols;
}
void mine_field_cpu::Reset()
{
	Init(_rows, _cols, _mines_count);
}
mine_cell mine_field_cpu::CheckCell(unsigned row, unsigned col, bool flag)
{
	if((row >= _rows) || (col >= _cols))
		return mine_cell(CELL_MINE); // mine
	if(_first_move)
	{
		_first_move = false;
		if(_mines[_cols * row + col])
			Relocate(row, col);
		return CheckCell(row, col);
	}
	if(_mines[_cols * row + col])
		return mine_cell(CELL_MINE); // mine
	return flag ? mine_cell(CELL_FLAG): mine_cell(CELL_SAFE, CheckNear(row, col));
}
void mine_field_cpu::Init(unsigned rows, unsigned cols, unsigned mines_count)
{
	if(!(rows * cols * mines_count))
		return;
	if(mines_count >= (rows * cols))
		return;
	_mines_count = mines_count;
	_rows = rows;
	_cols = cols;
	_first_move = true;
	if(_mines != nullptr)
		delete [] _mines;
	_mines = new bool[rows * cols];
	memset(_mines, 0, sizeof(bool) * rows * cols);
	_safe.resize(rows * cols);
	for(unsigned i = 0; i < rows * cols; i++)
		_safe[i] = i;
	for(unsigned i = 0; i < _mines_count; i++)
	{
		unsigned random_free = RandomFree();
		_mines[_safe[random_free]] = true;
		_safe.erase(_safe.begin()+random_free);
	}
}
int mine_field_cpu::CheckNear(unsigned row, unsigned col)
{
	int res = 0;
	for(int i = -1; i <= 1; i++)
	for(int j = -1; j <= 1; j++)
	{
		if((i == 0) && (j == 0))
			continue;
		if(CHECK_RANGE(row+i, col+j))
			res += _mines[_cols * (row+i) + (col+j)];
	}
	return res;
}
void mine_field_cpu::Relocate(unsigned row, unsigned col)
{
	unsigned random_free = RandomFree();
	_mines[_safe[random_free]] = true;
	_mines[row * _cols + col] = false;
	_safe.erase(_safe.begin()+random_free);
	_safe.push_back(row * _cols + col);
}
unsigned mine_field_cpu::RandomFree()
{
	unsigned rand_num;
	rand_s(&rand_num);
	return rand_num % _safe.size();
}
/*
	Miner class
*/
void miner::Init(mine_field* mines, std::vector<mine_cell> field)
{
	if(mines == nullptr)
		return;
	_mines = mines;
	_field.clear();
	_unknown.clear();
	_moves.clear();
	_changed.clear();
	_prepared_moves.clear();
	_rows = _mines->GetRows();
	_cols = _mines->GetCols();
	if(field.size() == _cols * _rows)
	{
		_field = field;
		for(unsigned i = 0; i < _rows * _cols; i++)
			if(_field[i].state == CELL_UNKNOWN)
			_unknown.push_back(i);
	}
	else
	{
		_unknown.resize(_rows * _cols); // same, but memory optimized
		_field.resize(_rows * _cols);
		for(unsigned i = 0; i < _rows * _cols; i++)
			_unknown[i] = i; 
	}
}
std::vector<miner_move> miner::PrepareMoves()
{
	_prepared_moves.clear();
	if(!_unknown.size())
		return _prepared_moves;
	for(auto cell_num: _changed)
	{
		if(_field[cell_num].state != CELL_SAFE)
			continue;
		unsigned row = cell_num / _cols;
		unsigned col = cell_num % _cols;
		std::vector<unsigned> near = GetNear(row, col);
		if(!near[CELL_UNKNOWN])
			continue;
		if(near[CELL_UNKNOWN] + near[CELL_FLAG] + near[CELL_MINE]== _field[cell_num].param) // if all unknown cells are mines
		{
			MarkAllNear(row, col, CELL_FLAG);
			continue;
		}
		if(near[CELL_FLAG] + near[CELL_MINE] == _field[cell_num].param) // if all unknown are safe
		{
			MarkAllNear(row, col, CELL_SAFE);
			continue;
		}
	}
#ifdef USE_PATTERNS
	if(!_prepared_moves.size())
	{
		for(auto& pat: patterns)
		{
			auto temp = CheckPattern(pat);
			_prepared_moves.insert(_prepared_moves.end(), temp.begin(), temp.end());
		}
	}
#endif
	if(!_prepared_moves.size())
		_prepared_moves.push_back(miner_move(ACTION_RANDOM));
#ifdef _DEBUG
			std::cout<< "Prepeared moves: "<< _prepared_moves.size() << std::endl; 
#endif
	return _prepared_moves;
}
int miner::Move(std::vector<miner_move>& moves)
{
	if(_mines == nullptr)
		return -1;
	_changed.clear();
	int res = -1;
	for(int i = 0; i < moves.size(); i++)
	{
		unsigned cell_num = moves[i].row * _cols + moves[i].col;
		unsigned row = moves[i].row;
		unsigned col = moves[i].col;
		if(!CHECK_RANGE(row, col))
			continue;
		switch(moves[i].action)
		{
		case ACTION_FLAG:
			if(_field[cell_num].state != CELL_UNKNOWN) // actions only over unlnown cells (for a now)
				continue;
#ifdef MARK_FLAG
			_field[cell_num] = _mines->CheckCell(row, col, true);
#else
			_field[cell_num] = mine_cell(CELL_FLAG);
#endif
#ifdef _DEBUG
			std::cout<< "Flag: "<< row << " " << col << std::endl; 
#endif
			break;
		case ACTION_RANDOM:
			cell_num = _unknown[RandomUnknown()];
			row = cell_num / _cols;
			col = cell_num % _cols;
#ifdef _DEBUG
			std::cout<< "Random! ";
#endif
		case ACTION_SAFE:
			if(_field[cell_num].state != CELL_UNKNOWN) // actions only over unlnown cells (for a now)
				continue;
			_field[cell_num] = _mines->CheckCell(row, col);
#ifdef _DEBUG
			std::cout<< "Safe: "<< row << " " << col <<std::endl; 
#endif
			if(_field[cell_num].state == CELL_MINE)
			{
				res = i;
				// here we can return from the function and stop the game
#ifdef _DEBUG
			std::cout<< "MINE!: "<< row << " " << col <<std::endl<<std::endl; 
#endif 
				return res;
			}
			break;
		}
		moves[i].result = _field[cell_num];
		if(_field[cell_num].state == CELL_UNKNOWN) // can't check, skip
			continue;
		if(_field[cell_num].state == CELL_ERROR) // some shit happens
			return res;
		auto unk = find(_unknown.begin(), _unknown.end(), cell_num); // some times it may check same cell twice (should be fixed)
		if(unk != _unknown.end())
			_unknown.erase(unk);
		_changed.insert(cell_num);
	}
	MarkAllNearChanged();
	return res;
}
int miner::Move()
{
	return Move(_prepared_moves);
}
void miner::MarkAllNear(unsigned row, unsigned col, cell_state state)
{
	for(int i = -1; i <= 1; i++)
	for(int j = -1; j <= 1; j++)
	{
		if((i == 0) && (j == 0))
			continue;
		if(	CHECK_RANGE(row+i, col+j) && 
			(_field[(row+i) * _cols + col+j].state == CELL_UNKNOWN)) //Only unknown cell can be marked (unknown == unmarked)
		{
			miner_move new_move;
			switch(state)
			{
			case CELL_FLAG:
				new_move.action = ACTION_FLAG;
				break;
			case CELL_SAFE:
				new_move.action = ACTION_SAFE;
				break;
			}
			new_move.row = row + i;
			new_move.col = col + j;
			_prepared_moves.push_back(new_move);
		}
	}
}
std::vector<unsigned> miner::GetNear(unsigned row, unsigned col)
{
	std::vector<unsigned> res;
	res.resize(CELL_MAX);
	for(int i = -1; i <= 1; i++)
	for(int j = -1; j <= 1; j++)
	{
		if((i == 0) && (j == 0))
			continue;
		if(CHECK_RANGE(row+i, col+j))
			res[_field[_cols * (row+i) + (col+j)].state]++;
		else
			res[CELL_SAFE]++;
	}
	return res;
}
void miner::MarkAllNearChanged()
{
	std::set<unsigned> old_changed = _changed;
	_changed.clear();
	for(auto cell_num: old_changed)
	{
		unsigned row = cell_num / _cols;
		unsigned col = cell_num % _cols;
		for(int i = -1; i <= 1; i++)
		for(int j = -1; j <= 1; j++)
		{
			if(	CHECK_RANGE(row+i, col+j) && 
				(_field[(row+i) * _cols + col+j].state == CELL_SAFE)) // only safe cells can be proceed
				_changed.insert((row+i) * _cols + col+j);
		}
	}
}
unsigned miner::RandomUnknown()
{
	unsigned rand_num;
	rand_s(&rand_num);
	return rand_num % _unknown.size();
}
std::vector<miner_move> miner::CheckPattern(field_pattern pattern)
{
	std::vector<miner_move> res;
	if(!CheckPatternValid(pattern))
		return res;
	cv::Size field_size(_cols, _rows);
	for(unsigned j = 0; j < 2; j++)
	{
		for(unsigned i = 0; i < 4; i++)
		{
			std::vector<cv::Point> offests = ::CheckPattern(_field, field_size, pattern);
			for(auto offset: offests)
			{
				for(auto move: pattern.moves)
				{
					move.row += offset.y;
					move.col += offset.x;
					res.push_back(move);
				}
			}
			pattern = Rotate90(pattern);
		}
		pattern = MirrorHor(pattern); // last time it will be useless
	}
	return res;
}
std::vector<cv::Point> CheckPattern(const std::vector<mine_cell>& field, cv::Size field_size, field_pattern& pattern)
{
	std::vector<cv::Point> res;
	if((pattern.field_part.size() / pattern.cols > field_size.height + 2) || 
		(pattern.cols > field_size.width + 2)) // maximum size is field_size with 1-cell border (both sides, so +2)
		return res;
	int bottom_limit = field_size.height - pattern.field_part.size()/pattern.cols + 1; // one cell over border (can be CELL_BORDER)
	int right_limit = field_size.width - pattern.cols + 1; // one cell over border (can be CELL_BORDER)
	for(int i = -1; i < bottom_limit; i++)
	for(int j = -1; j < right_limit; j++)
	{
		cv::Point temp(j, i);
		if(CheckPattern(field, field_size, pattern, temp))
		{
#ifdef _DEBUG
			std::cout << "Pattern fit: " << temp.y << " " << temp.x << std::endl;
#endif
			res.push_back(temp);
		}
	}

	return res;
}
bool CheckPattern(const std::vector<mine_cell>& field, cv::Size field_size, field_pattern& pattern, cv::Point point)
{
	bool res = false;
	cv::Size pattern_size(pattern.cols, pattern.field_part.size()/pattern.cols);
	field_pattern minefield_pattern;
	minefield_pattern.field_part.resize(pattern.field_part.size());
	minefield_pattern.cols = pattern.cols;
	for(unsigned i = 0; i < pattern_size.height; i++)
	for(unsigned j = 0; j < pattern_size.width; j++)
	{
		int row_num = i + point.y; // signed type because it can be out of border (becomes CELL_BORDER)
		int col_num = j + point.x;
		int field_cell_num = row_num * field_size.width + col_num;
		unsigned pattern_cell_num = i * pattern_size.width + j;
		if(IN_RANGE(row_num, 0, field_size.height) && IN_RANGE(col_num, 0, field_size.width))
		{
			minefield_pattern.field_part[pattern_cell_num].push_back(field[field_cell_num]);
		}
		else
		{
			minefield_pattern.field_part[pattern_cell_num].push_back(mine_cell(CELL_BORDER));
		}
	}
	return ComparePatterns(pattern, minefield_pattern);
}
bool ComparePatterns(field_pattern& pattern1, field_pattern& pattern2)
{
	if(pattern1.field_part.size() != pattern2.field_part.size())
		return false;
	bool move_res = false;
	for(auto move: pattern1.moves)
	{
		unsigned cell_num = pattern2.cols * move.row + move.col;
		cell_state cur_cell_state = pattern2.field_part[cell_num][0].state;
		switch (move.action) // if cell already in target state (CELL_SAFE, CELL_FLAG or even CELL_BORDER) we should ignore that pattern (actions useless)
		{
		case ACTION_SAFE: // should be CELL_UNKNOWN (can't unflag and mark CELL_SAFE now)
			if(cur_cell_state == CELL_UNKNOWN)
				move_res = true;
			break;
		case ACTION_FLAG:
			if(cur_cell_state == CELL_UNKNOWN)
				move_res = true;
			break;
		}
	}
	if(!move_res)
		return false;
	for(unsigned i = 0; i < pattern1.field_part.size(); i++)
	{
		int check = pattern1.checks[i];
		bool not = check < 0 ? true : false;
		check = abs(check);
		bool temp_res = false;
		for(auto cell_variant: pattern1.field_part[i])
		{
			bool check_res[2] = {false, false};
			if(check & FP_STATE) // check state
			{
			
				if(cell_variant.state == pattern2.field_part[i][0].state)
					check_res[0] = true;
			}
			else
				check_res[0] = true;
			if(check & FP_PARAM) // check param (why?)
			{
				if(cell_variant.param == pattern2.field_part[i][0].param)
					check_res[1] = true;
			}
			else
				check_res[1] = true;
			if(check_res[0] & check_res[1])
			{
				temp_res = true;
				break;
			}
		}
		temp_res = not ? !temp_res : temp_res;
		if(!temp_res)
			return false;
	}
	return false;
}

/*
Pattern 1 1
field part
	S/B   S/B   S/B
	U/S/B 1     U
	U/S/B 1     U
	U/S	  U/S   U/S // one of them shouldn't be safe
checks
	FP_STATE	FP_STATE			FP_STATE
	FP_STATE	FP_STATE | FP_PARAM	FP_STATE
	FP_STATE	FP_STATE | FP_PARAM	FP_STATE
	FP_STATE	FP_STATE			FP_STATE
moves
	0     0     0
	0     0     0
	0     0     0
	S     S     S
	*/
field_pattern pattern_1_1 = {	
	{	//field
{mine_cell(CELL_SAFE), mine_cell(CELL_BORDER)}, {mine_cell(CELL_SAFE), mine_cell(CELL_BORDER)}, {mine_cell(CELL_SAFE), mine_cell(CELL_BORDER)},
{mine_cell(CELL_SAFE), mine_cell(CELL_BORDER), mine_cell()}, {mine_cell(CELL_SAFE, 1)}, {mine_cell()},
{mine_cell(CELL_SAFE), mine_cell(CELL_BORDER), mine_cell()}, {mine_cell(CELL_SAFE, 1)}, {mine_cell()},
{mine_cell(CELL_SAFE), mine_cell()}, {mine_cell(CELL_SAFE), mine_cell()}, {mine_cell(CELL_SAFE), mine_cell()},
	},
	{	//checks	
		FP_STATE,	FP_STATE,			FP_STATE,	
		FP_STATE,	FP_STATE | FP_PARAM,FP_STATE,
		FP_STATE,	FP_STATE | FP_PARAM,FP_STATE,
		FP_STATE,	FP_STATE,			FP_STATE
	},
	3,	// cols
	{	//moves
		miner_move(ACTION_SAFE, 3, 0),
		miner_move(ACTION_SAFE, 3, 1),
		miner_move(ACTION_SAFE, 3, 2)
	}
};

std::vector<field_pattern> patterns = {pattern_1_1};