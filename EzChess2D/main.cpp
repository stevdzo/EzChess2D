#include <cstring>
#include <iostream>
#include <string>

#include "ChessCore.h"
#include "GlutCallbacks.h"

int main(int _Argc, char** _Argv)
{
	// "EzChess2D.exe --perft"
	std::string StartFen;

	for (int Index = 1; Index < _Argc; ++Index)
	{
		if (std::strcmp(_Argv[Index], "--perft") == 0)
		{
			return Chess::RunSelfTest() ? 0 : 1;
		}

		if (std::strcmp(_Argv[Index], "--fen") == 0 && Index + 1 < _Argc)
		{
			StartFen = _Argv[++Index];

			Chess::Position Parsed;
			if (!Chess::Position::FromFen(StartFen, Parsed))
			{
				std::cout << "Broken FEN: " << StartFen << std::endl;
				return 1;
			}
		}
	}

	return GlutMain(_Argc, _Argv, StartFen);
}
