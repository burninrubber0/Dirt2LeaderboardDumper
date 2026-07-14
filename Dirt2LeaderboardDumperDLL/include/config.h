#pragma once

#include <Windows.h>

struct Config
{
	// Title ID of the game to request leaderboards for.
	DWORD titleId = 0;
	// (Inclusive) ID of the first leaderboard to dump.
	// If greater than the number of leaderboards, nothing is dumped.
	// If 0 or invalid, this is set to 1.
	DWORD startLeaderboard = 0;
	// (Inclusive) ID of the last leaderboard to dump.
	// If 0, invalid, or greater than the number of leaderboards, this is set to the ID of the last overall leaderboard.
	DWORD endLeaderboard = 0;
	// (Inclusive) Rank to begin dumping at.
	// If greater than the number of rows, nothing is dumped.
	// If 0 or invalid, this is set to 1.
	DWORD startRank = 0;
	// (Inclusive) Rank to stop dumping at.
	// If 0, invalid, or greater than the number of rows, this is set to the last overall rank.
	DWORD endRank = 0;
};
