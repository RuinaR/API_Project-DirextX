#include "pch.h"
#include "game.h"

string MapTypeToString(MapType type)
{
	switch (type)
	{
	case MapType::None:
		return "NONE";
		break;
	case MapType::Block:
		return "BLOCK";
		break;
	case MapType::Player:
		return "PLAYER";
		break;
	case MapType::DefaultMon:
		return "DEFAULT_MON";
		break;
	case MapType::SwordMon:
		return "SWORD_MON";
		break;
	case MapType::StoneMon:
		return "STONE_MON";
		break;
	case MapType::Door:
		return "DOOR";
		break;
	}
	return "ERR";
}