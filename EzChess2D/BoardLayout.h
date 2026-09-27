#ifndef _BOARD_LAYOUT_H
#define _BOARD_LAYOUT_H

#include "Vec2.h"

namespace Layout {

constexpr float WorldWidth  = 600.0f;
constexpr float WorldHeight = 750.0f;

constexpr float TileSize     = 64.0f;
constexpr float BoardOriginX = 44.0f;
constexpr float BoardOriginY = 150.0f;
constexpr float BoardSize    = TileSize * 8.0f;

constexpr float BoardCenterX = BoardOriginX + BoardSize * 0.5f;
constexpr float BoardCenterY = BoardOriginY + BoardSize * 0.5f;

constexpr float RankLabelX      = 26.0f;
constexpr float FileLabelY      = 132.0f;
constexpr float CapturedBlackY  = 108.0f;
constexpr float CapturedWhiteY  = 72.0f;
constexpr float CapturedSize    = 30.0f;
constexpr float CapturedOriginX = 44.0f;

constexpr float StatusY = 682.0f;
constexpr float ResultY = 714.0f;
constexpr float HintY   = 22.0f;

constexpr float PromotionCellSize = 64.0f;
constexpr float PromotionPanelW   = PromotionCellSize * 4.0f + 16.0f;
constexpr float PromotionPanelH   = PromotionCellSize + 16.0f;

inline Vec2 SquareCenter(int _File, int _Rank)
{
	return Vec2(BoardOriginX + _File * TileSize + TileSize * 0.5f,
	            BoardOriginY + _Rank * TileSize + TileSize * 0.5f);
}

inline Vec2 SquareCenter(int _Square)
{
	return SquareCenter(_Square & 7, _Square >> 3);
}

inline int SquareAt(const Vec2& _World)
{
	const float LocalX = _World.X - BoardOriginX;
	const float LocalY = _World.Y - BoardOriginY;

	if (LocalX < 0.0f || LocalY < 0.0f || LocalX >= BoardSize || LocalY >= BoardSize)
	{
		return -1;
	}

	const int File = static_cast<int>(LocalX / TileSize);
	const int Rank = static_cast<int>(LocalY / TileSize);
	return Rank * 8 + File;
}

inline Vec2 PromotionChoiceCenter(int _Index)
{
	const float FirstX = BoardCenterX - PromotionCellSize * 1.5f;
	return Vec2(FirstX + _Index * PromotionCellSize, BoardCenterY);
}

struct ViewportRect {
	int X;
	int Y;
	int Width;
	int Height;
};

inline ViewportRect Letterbox(int _WindowWidth, int _WindowHeight)
{
	if (_WindowWidth <= 0 || _WindowHeight <= 0)
	{
		return ViewportRect{ 0, 0, 1, 1 };
	}

	const float ScaleX = _WindowWidth / WorldWidth;
	const float ScaleY = _WindowHeight / WorldHeight;
	const float Scale  = (ScaleX < ScaleY) ? ScaleX : ScaleY;

	const int Width  = static_cast<int>(WorldWidth * Scale);
	const int Height = static_cast<int>(WorldHeight * Scale);

	return ViewportRect{ (_WindowWidth - Width) / 2, (_WindowHeight - Height) / 2, Width, Height };
}

inline Vec2 MouseToWorld(int _MouseX, int _MouseY, int _WindowWidth, int _WindowHeight)
{
	const ViewportRect View = Letterbox(_WindowWidth, _WindowHeight);
	if (View.Width <= 0 || View.Height <= 0)
	{
		return Vec2(-1.0f, -1.0f);
	}

	const float FlippedY = static_cast<float>(_WindowHeight - _MouseY);

	return Vec2((_MouseX - View.X) * WorldWidth / View.Width,
	            (FlippedY - View.Y) * WorldHeight / View.Height);
}

}
#endif
