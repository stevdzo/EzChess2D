#include "SceneManager.h"

#include "BoardLayout.h"
#include "PieceAtlas.h"

using namespace Chess;

namespace {

const Vec3 LightTile   (1.0f,    0.807f, 0.619f);
const Vec3 DarkTile    (0.8196f, 0.545f, 0.278f);
const Vec3 SelectColor (1.0f,    1.0f,   1.0f);
const Vec3 MoveColor   (0.1f,    0.75f,  0.2f);
const Vec3 LastMoveColor(0.95f,  0.85f,  0.25f);
const Vec3 CheckColor  (0.9f,    0.15f,  0.15f);
const Vec3 TextColor   (0.93f,   0.93f,  0.90f);
const Vec3 DimTextColor(0.62f,   0.60f,  0.56f);
const Vec3 PanelColor  (0.12f,   0.12f,  0.14f);

const Vec2 TileVec(Layout::TileSize, Layout::TileSize);

}

void SceneManager::Display(const GameManager& _Manager) const
{
	RenderTiles(_Manager);
	RenderLastMove(_Manager);
	RenderCheck(_Manager);
	RenderSelection(_Manager);
	RenderPieces(_Manager);
	RenderMoveMarkers(_Manager);
	RenderCoordinates();
	RenderCapturedPieces(_Manager);
	RenderTextOverlay(_Manager);
	RenderPromotionPanel(_Manager);
}

void SceneManager::RenderTiles(const GameManager& /*_Manager*/) const
{
	for (int Square = 0; Square < BoardSquares; ++Square)
	{
		const bool bIsLight = ((FileOf(Square) + RankOf(Square)) & 1) != 0;
		Draw.RenderFillRect(Layout::SquareCenter(Square), TileVec, bIsLight ? LightTile : DarkTile);
	}
}

void SceneManager::RenderLastMove(const GameManager& _Manager) const
{
	if (!_Manager.Game().HasLastMove())
	{
		return;
	}

	const Move Last = _Manager.Game().LastMove();
	Draw.RenderFillRect(Layout::SquareCenter(Last.From), TileVec, LastMoveColor, 0.35f);
	Draw.RenderFillRect(Layout::SquareCenter(Last.To), TileVec, LastMoveColor, 0.45f);
}

void SceneManager::RenderCheck(const GameManager& _Manager) const
{
	const int KingSquare = _Manager.CheckedKingSquare();
	if (KingSquare < 0)
	{
		return;
	}

	Draw.RenderFillRect(Layout::SquareCenter(KingSquare), TileVec, CheckColor, 0.55f);
}

void SceneManager::RenderSelection(const GameManager& _Manager) const
{
	if (_Manager.SelectedSquare() < 0)
	{
		return;
	}

	Draw.RenderWireframeRect(Layout::SquareCenter(_Manager.SelectedSquare()), TileVec, SelectColor, 3.0f);
}

void SceneManager::RenderPieceAt(Piece _Piece, Vec2 _Center, float _Size) const
{
	if (_Piece.IsEmpty())
	{
		return;
	}

	const int Index = PieceAtlas::IndexFor(_Piece.Type, _Piece.Side);
	Draw.RenderSprite(PieceAtlas::Texture(), Index, PieceAtlas::Frames(), _Center, Vec2(_Size, _Size));
}

void SceneManager::RenderPieces(const GameManager& _Manager) const
{
	const Position& Pos = _Manager.Game().Pos();

	for (int Square = 0; Square < BoardSquares; ++Square)
	{
		RenderPieceAt(Pos.At(Square), Layout::SquareCenter(Square), Layout::TileSize);
	}
}

void SceneManager::RenderMoveMarkers(const GameManager& _Manager) const
{
	if (_Manager.SelectedSquare() < 0)
	{
		return;
	}

	for (const Move& Candidate : _Manager.SelectedMoves())
	{
		const Vec2 Center = Layout::SquareCenter(Candidate.To);

		if (Candidate.Flags & MF_Capture)
		{
			Draw.RenderRing(Center, Layout::TileSize * 0.44f, 5.0f, MoveColor, 0.85f);
		}
		else
		{
			Draw.RenderFillCircle(Center, Layout::TileSize * 0.16f, MoveColor, 0.8f);
		}
	}
}

void SceneManager::RenderCoordinates() const
{
	for (int File = 0; File < 8; ++File)
	{
		const std::string Label(1, static_cast<char>('a' + File));
		const Vec2 Center(Layout::SquareCenter(File, 0).X, Layout::FileLabelY);
		Draw.RenderTextCentered(Center, Label, DimTextColor, GLUT_BITMAP_HELVETICA_12);
	}

	for (int Rank = 0; Rank < 8; ++Rank)
	{
		const std::string Label(1, static_cast<char>('1' + Rank));
		const Vec2 Center(Layout::RankLabelX, Layout::SquareCenter(0, Rank).Y - 5.0f);
		Draw.RenderTextCentered(Center, Label, DimTextColor, GLUT_BITMAP_HELVETICA_12);
	}
}

void SceneManager::RenderCapturedPieces(const GameManager& _Manager) const
{
	const std::vector<Piece>& LostBlack = _Manager.CapturedBlack();
	for (size_t Index = 0; Index < LostBlack.size(); ++Index)
	{
		const Vec2 Center(Layout::CapturedOriginX + Index * (Layout::CapturedSize * 0.72f),
		                  Layout::CapturedBlackY);
		RenderPieceAt(LostBlack[Index], Center, Layout::CapturedSize);
	}

	const std::vector<Piece>& LostWhite = _Manager.CapturedWhite();
	for (size_t Index = 0; Index < LostWhite.size(); ++Index)
	{
		const Vec2 Center(Layout::CapturedOriginX + Index * (Layout::CapturedSize * 0.72f),
		                  Layout::CapturedWhiteY);
		RenderPieceAt(LostWhite[Index], Center, Layout::CapturedSize);
	}
}

void SceneManager::RenderTextOverlay(const GameManager& _Manager) const
{
	Draw.RenderTextCentered(Vec2(Layout::BoardCenterX, Layout::StatusY),
	                        _Manager.StatusLine(), TextColor);

	const std::string Result = _Manager.ResultLine();
	if (!Result.empty())
	{
		Draw.RenderTextCentered(Vec2(Layout::BoardCenterX, Layout::ResultY), Result, LastMoveColor);
	}

	Draw.RenderTextCentered(Vec2(Layout::BoardCenterX, Layout::HintY),
	                        _Manager.HintLine(), DimTextColor, GLUT_BITMAP_HELVETICA_12);
}

void SceneManager::RenderPromotionPanel(const GameManager& _Manager) const
{
	if (!_Manager.IsPromotionPending())
	{
		return;
	}

	Draw.RenderFillRect(Vec2(Layout::BoardCenterX, Layout::BoardCenterY),
	                    Vec2(Layout::BoardSize, Layout::BoardSize), PanelColor, 0.6f);

	Draw.RenderFillRect(Vec2(Layout::BoardCenterX, Layout::BoardCenterY),
	                    Vec2(Layout::PromotionPanelW, Layout::PromotionPanelH), PanelColor, 0.95f);

	Draw.RenderWireframeRect(Vec2(Layout::BoardCenterX, Layout::BoardCenterY),
	                         Vec2(Layout::PromotionPanelW, Layout::PromotionPanelH), SelectColor, 2.0f);

	const Color Side = _Manager.Game().Pos().SideToMove();
	const PieceType Choices[4] = {
		PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight
	};

	for (int Index = 0; Index < 4; ++Index)
	{
		RenderPieceAt(Piece{ Choices[Index], Side },
		              Layout::PromotionChoiceCenter(Index), Layout::PromotionCellSize);
	}
}
