#include "GameManager.h"

#include "BoardLayout.h"

using namespace Chess;

namespace {

const int InitialCount[7] = {
	0,   // None
	8,   // Pawn
	2,   // Knight
	2,   // Bishop
	2,   // Rook
	1,   // Queen
	1    // King
};

const PieceType DisplayOrder[5] = {
	PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight, PieceType::Pawn
};

}

GameManager::GameManager()
{
	NewGame();
}

void GameManager::SetStartFen(const std::string& _Fen)
{
	StartFen = _Fen;
	NewGame();
}

void GameManager::NewGame()
{
	if (StartFen.empty() || !Core.ResetFromFen(StartFen))
	{
		Core.Reset();
	}

	ClearSelection();
	RebuildCaptured();
}

void GameManager::ClearSelection()
{
	SelectedSq = -1;
	MovesForSelected.clear();
	bPromotionPending = false;
	PromotionOptions.clear();
}

void GameManager::Select(int _Square)
{
	SelectedSq = _Square;
	MovesForSelected.clear();

	Move Buffer[MaxMoves];
	const int Count = Core.MovesFrom(_Square, Buffer);

	MovesForSelected.assign(Buffer, Buffer + Count);
}

void GameManager::PlayMove(const Move& _Move)
{
	Core.Make(_Move);
	ClearSelection();
	RebuildCaptured();
}

void GameManager::OnClick(const Vec2& _World)
{
	if (bPromotionPending)
	{
		OnPromotionClick(_World);
		return;
	}

	if (Core.IsOver())
	{
		return;
	}

	const int Square = Layout::SquareAt(_World);
	if (Square < 0)
	{
		ClearSelection();
		return;
	}

	if (SelectedSq >= 0)
	{
		std::vector<Move> Matching;
		for (const Move& Candidate : MovesForSelected)
		{
			if (Candidate.To == Square)
			{
				Matching.push_back(Candidate);
			}
		}

		if (Matching.size() == 1)
		{
			PlayMove(Matching.front());
			return;
		}

		if (Matching.size() > 1)
		{
			bPromotionPending = true;
			PromotionOptions  = Matching;
			return;
		}
	}

	const Piece Clicked = Core.Pos().At(Square);
	if (Clicked.Is(Core.Pos().SideToMove()))
	{
		if (SelectedSq == Square)
		{
			ClearSelection();
		}
		else
		{
			Select(Square);
		}
		return;
	}

	ClearSelection();
}

void GameManager::OnPromotionClick(const Vec2& _World)
{
	const float HalfW = Layout::PromotionPanelW * 0.5f;
	const float HalfH = Layout::PromotionPanelH * 0.5f;

	const bool bInsidePanel =
		_World.X >= Layout::BoardCenterX - HalfW && _World.X <= Layout::BoardCenterX + HalfW &&
		_World.Y >= Layout::BoardCenterY - HalfH && _World.Y <= Layout::BoardCenterY + HalfH;

	if (!bInsidePanel)
	{
		ClearSelection();
		return;
	}

	for (int Index = 0; Index < 4; ++Index)
	{
		const Vec2 Center = Layout::PromotionChoiceCenter(Index);
		const float Half  = Layout::PromotionCellSize * 0.5f;

		if (_World.X < Center.X - Half || _World.X > Center.X + Half)
		{
			continue;
		}

		const PieceType Chosen = (Index == 0) ? PieceType::Queen
		                       : (Index == 1) ? PieceType::Rook
		                       : (Index == 2) ? PieceType::Bishop
		                                      : PieceType::Knight;

		for (const Move& Candidate : PromotionOptions)
		{
			if (Candidate.Promotion == Chosen)
			{
				PlayMove(Candidate);
				return;
			}
		}
	}
}

void GameManager::OnKey(unsigned char _Key)
{
	switch (_Key)
	{
	case 'r':
	case 'R':
		NewGame();
		break;

	case 'u':
	case 'U':
		if (bPromotionPending)
		{
			ClearSelection();
			break;
		}
		if (Core.Undo())
		{
			ClearSelection();
			RebuildCaptured();
		}
		break;

	default:
		break;
	}
}

bool GameManager::IsTargetOf(int _Square, bool& _bOutIsCapture) const
{
	for (const Move& Candidate : MovesForSelected)
	{
		if (Candidate.To != _Square)
		{
			continue;
		}

		_bOutIsCapture = (Candidate.Flags & MF_Capture) != 0;
		return true;
	}

	_bOutIsCapture = false;
	return false;
}

int GameManager::CheckedKingSquare() const
{
	const Color Side = Core.Pos().SideToMove();
	if (!Core.Pos().IsInCheck(Side))
	{
		return -1;
	}
	return Core.Pos().KingSquare(Side);
}

void GameManager::RebuildCaptured()
{
	LostWhite.clear();
	LostBlack.clear();

	int Present[2][7] = { { 0 } };

	for (int Square = 0; Square < BoardSquares; ++Square)
	{
		const Piece Current = Core.Pos().At(Square);
		if (Current.IsEmpty())
		{
			continue;
		}

		const int SideIndex = (Current.Side == Color::White) ? 0 : 1;
		++Present[SideIndex][static_cast<int>(Current.Type)];
	}

	for (const PieceType Type : DisplayOrder)
	{
		const int TypeIndex = static_cast<int>(Type);

		const int MissingWhite = InitialCount[TypeIndex] - Present[0][TypeIndex];
		for (int Index = 0; Index < MissingWhite; ++Index)
		{
			LostWhite.push_back(Piece{ Type, Color::White });
		}

		const int MissingBlack = InitialCount[TypeIndex] - Present[1][TypeIndex];
		for (int Index = 0; Index < MissingBlack; ++Index)
		{
			LostBlack.push_back(Piece{ Type, Color::Black });
		}
	}
}

std::string GameManager::StatusLine() const
{
	if (Core.IsOver())
	{
		return "Game over";
	}

	std::string Line = (Core.Pos().SideToMove() == Color::White) ? "White move" : "Black move";
	Line += "   (turn " + std::to_string(Core.MoveNumber()) + ")";

	if (Core.Pos().IsInCheck(Core.Pos().SideToMove()))
	{
		Line += "   -   MATE!";
	}

	return Line;
}

std::string GameManager::ResultLine() const
{
	return ResultText(Core.Result());
}

std::string GameManager::HintLine() const
{
	if (bPromotionPending)
	{
		return "Choose the piece to promote";
	}
	return "R = new game     U = undo turn     ESC = exit";
}
