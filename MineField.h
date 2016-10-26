#pragma once
#include <vector>
#include <set>

#define IN_RANGE(val, _min, _max) ((_min) <= (val)) &&  ((val) <= (_max))
#define CHECK_RANGE(row_num, col_num) (IN_RANGE((row_num), 0, _rows-1) && IN_RANGE((col_num), 0, _cols-1))

enum cell_state
{
	CELL_UNKNOWN = 0,
	CELL_FLAG,
	CELL_SAFE,
	CELL_MINE,
	CELL_MAX
};
struct mine_cell
{
	mine_cell(cell_state init_state = CELL_UNKNOWN, int init_param = -1);
	cell_state state;
	int param;
};
enum miner_action
{
	ACTION_FLAG = 0,
	ACTION_SAFE,
	ACTION_RANDOM
};
struct miner_move
{
	miner_move(miner_action act = ACTION_RANDOM, unsigned r = 0, unsigned c = 0);
	miner_action action;
	unsigned row;
	unsigned col;
	mine_cell result;
};
class mine_field
{
public:
	virtual ~mine_field(){}
	virtual mine_cell CheckCell(unsigned row, unsigned col, bool flag = false) = 0;
	virtual unsigned GetRows() = 0;
	virtual unsigned GetCols() = 0;
	virtual void Reset() = 0;
protected:
};

class mine_field_cpu: public mine_field
{
public:
	mine_field_cpu();
	virtual ~mine_field_cpu();
	virtual unsigned GetRows();
	virtual unsigned GetCols();
	virtual void Reset();
	virtual mine_cell CheckCell(unsigned row, unsigned col, bool flag = false);
	void Init(unsigned rows, unsigned cols, unsigned mines_count);
protected:
	int CheckNear(unsigned row, unsigned col); // get number of near mines
	void Relocate(unsigned row, unsigned col); // relocate mine
	unsigned RandomFree();
protected:
	bool* _mines;
	unsigned _rows;
	unsigned _cols;
	unsigned _mines_count;
	bool _first_move;
	std::vector<unsigned> _safe;
};

class miner
{
public:
	virtual ~miner(){}
	virtual void Init(mine_field* mines, std::vector<mine_cell> field = std::vector<mine_cell>());
	std::vector<miner_move> PrepareMoves();
	virtual int Move(std::vector<miner_move>& moves);
	int Move();
protected:
	void MarkAllNear(unsigned row, unsigned col, cell_state state);
	std::vector<unsigned> GetNear(unsigned row, unsigned col);
	void MarkAllNearChanged();
	unsigned RandomUnknown();
protected:
	mine_field* _mines;
	unsigned _rows;
	unsigned _cols;
	std::vector<mine_cell> _field;
	std::vector<unsigned> _unknown;
	std::vector<miner_move> _moves;
	std::set<unsigned>	_changed;
	std::vector<miner_move> _prepared_moves;
};