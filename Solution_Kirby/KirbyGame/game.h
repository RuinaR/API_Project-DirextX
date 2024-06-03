#pragma once

using namespace std;

#define UNITSIZE 100
#define MAXUNITCNT 100

#define TAG_LAND "land"
#define TAG_ATTACK "attack"
#define TAG_PLAYER "player"
#define TAG_CHANGE "change"
#define TAG_HIT "hit"
#define TAG_MONSTER "monster"
#define TAG_DOOR "door"

#define FILE_MAP "map"

#define PLAYER_DEFAULT_X 5
#define PLAYER_DEFAULT_Y 5

enum class Arrow
{
	left,
	right,
	max
};

enum class PlayerAState
{
	idle,
	walk,
	jump,
	run,
	fly,
	eat_idle,
	eat_move,
	eat_jump,
	change,
	eat,
	eating,
	hit,
	attack,
	max
};

enum class PlayerMode
{
	mDefault,
	mSword,
	mStone,
	max
};

enum class MapType
{
	None = 0,
	Player = 1,
	Block = 2,
	DefaultMon = 3,
	SwordMon = 4,
	StoneMon = 5,
	Door = 6,
	max
};

struct UndoRedoData
{
	int x;
	int y;
	MapType type;
	bool isErase;
};

string MapTypeToString(MapType type);

