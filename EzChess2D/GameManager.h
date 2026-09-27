#ifndef _GAME_MANAGER_H
#define _GAME_MANAGER_H

#include <string>
#include <vector>

#include "ChessCore.h"
#include "Vec2.h"

class GameManager {

public:

	GameManager();

	void NewGame();
	void SetStartFen(const std::string& _Fen);

	void OnClick(const Vec2& _World);
	void OnKey(unsigned char _Key);

	const Chess::Game& Game() const { return Core; }

	int                             SelectedSquare() const { return SelectedSq; }
	const std::vector<Chess::Move>& SelectedMoves() const  { return MovesForSelected; }
	bool                            IsTargetOf(int _Square, bool& _bOutIsCapture) const;

	bool                            IsPromotionPending() const { return bPromotionPending; }
	const std::vector<Chess::Move>& PromotionMoves() const     { return PromotionOptions; }

	const std::vector<Chess::Piece>& CapturedWhite() const { return LostWhite; }
	const std::vector<Chess::Piece>& CapturedBlack() const { return LostBlack; }

	int CheckedKingSquare() const;

	std::string StatusLine() const;
	std::string ResultLine() const;
	std::string HintLine() const;

private:

	void Select(int _Square);
	void ClearSelection();
	void PlayMove(const Chess::Move& _Move);
	void OnPromotionClick(const Vec2& _World);
	void RebuildCaptured();

	Chess::Game Core;
	std::string StartFen;

	int                      SelectedSq = -1;
	std::vector<Chess::Move> MovesForSelected;

	bool                     bPromotionPending = false;
	std::vector<Chess::Move> PromotionOptions;

	std::vector<Chess::Piece> LostWhite;
	std::vector<Chess::Piece> LostBlack;
};

#endif
