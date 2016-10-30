#pragma once
#include <vector>
#include <set>
#include <opencv2/core.hpp>

#define IN_RANGE(val, _min, _max) ((_min) <= (val)) &&  ((val) <= (_max))
#define CHECK_RANGE(row_num, col_num) (IN_RANGE((row_num), 0, _rows-1) && IN_RANGE((col_num), 0, _cols-1))

//#define MARK_FLAG

enum cell_state
{
	CELL_UNKNOWN = 0,
	CELL_FLAG,
	CELL_SAFE,
	CELL_MINE,
	CELL_ERROR,
	CELL_BORDER,
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
	ACTION_NONE = 0,
	ACTION_FLAG,
	ACTION_SAFE,
	ACTION_RANDOM
};
struct miner_move
{
	miner_move(miner_action act = ACTION_NONE, unsigned r = 0, unsigned c = 0);
	miner_action action;
	int row;
	int col;
	mine_cell result;
};

#define FP_STATE		0x01
#define FP_PARAM		0x02

struct field_pattern
{
	std::vector<std::vector<mine_cell>> field_part;
	std::vector<int> checks; // negative == not, -1 * (FP_STATE) = not specified state
	unsigned cols;
	std::vector<miner_move> moves;
};
bool CheckPatternValid(const field_pattern& pattern);
field_pattern Rotate90(field_pattern pattern); // CW rotate
field_pattern MirrorHor(field_pattern pattern); // Horizontal mirror
field_pattern CutPattern(	std::vector<mine_cell> field, cv::Size field_size, 
							cv::Size pattern_size, cv::Point point);
class mine_field
{
public:
	virtual ~mine_field(){}
	virtual mine_cell CheckCell(unsigned row, unsigned col, bool flag = false) = 0;
	virtual unsigned GetRows() = 0;
	virtual unsigned GetCols() = 0;
	virtual void Reset() = 0;
	virtual std::vector<mine_cell> GetFieldView();
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
	virtual void Init(mine_field* mines);
	std::vector<miner_move> PrepareMoves();
	virtual int Move(std::vector<miner_move>& moves);
	int Move();
protected:
	void MarkAllNear(unsigned row, unsigned col, cell_state state);
	std::vector<unsigned> GetNear(unsigned row, unsigned col);
	void MarkAllNearChanged();
	unsigned RandomUnknown();
	std::vector<miner_move> CheckPattern(field_pattern pattern);
protected:
	mine_field* _mines;
	unsigned _rows;
	unsigned _cols;
	std::vector<mine_cell> _field;
	std::vector<unsigned> _unknown;
	std::vector<miner_move> _moves;
	std::set<unsigned>	_changed;
	std::set<unsigned>	_vanguard;
	std::vector<miner_move> _prepared_moves;
};

std::vector<cv::Point> CheckPattern(const std::vector<mine_cell>& field, cv::Size field_size, field_pattern& pattern);
std::vector<cv::Point> CheckPatternAround(const std::vector<mine_cell>& field, cv::Size field_size, field_pattern& pattern, cv::Point point);
bool CheckPattern(const std::vector<mine_cell>& field, cv::Size field_size, field_pattern& pattern, cv::Point point);
bool ComparePatterns(field_pattern& pattern1, field_pattern& pattern2);

extern std::vector<field_pattern> patterns;