#ifndef _CHESS_CORE_H
#define _CHESS_CORE_H

#include <cstdint>
#include <string>
#include <vector>

namespace Chess {

constexpr int BoardSquares = 64;
constexpr int MaxMoves     = 256;
constexpr int NoSquare     = -1;

enum class Color : uint8_t { White, Black, None };

enum class PieceType : uint8_t { None, Pawn, Knight, Bishop, Rook, Queen, King };

inline Color Opposite(Color _Side) { return _Side == Color::White ? Color::Black : Color::White; }

struct Piece {
	PieceType Type = PieceType::None;
	Color     Side = Color::None;

	bool IsEmpty() const { return Type == PieceType::None; }
	bool Is(Color _Side) const { return Type != PieceType::None && Side == _Side; }
};

constexpr int  FileOf(int _Square)            { return _Square & 7; }
constexpr int  RankOf(int _Square)            { return _Square >> 3; }
constexpr int  SquareOf(int _File, int _Rank) { return _Rank * 8 + _File; }
constexpr bool IsInside(int _File, int _Rank) { return _File >= 0 && _File < 8 && _Rank >= 0 && _Rank < 8; }
constexpr bool IsSquare(int _Square)          { return _Square >= 0 && _Square < BoardSquares; }

std::string SquareName(int _Square);
int         SquareFromName(const std::string&);

enum CastleRight : uint8_t {
	CR_None       = 0,
	CR_WhiteKing  = 1 << 0,
	CR_WhiteQueen = 1 << 1,
	CR_BlackKing  = 1 << 2,
	CR_BlackQueen = 1 << 3,
	CR_All        = 0x0F
};

enum MoveFlag : uint8_t {
	MF_None        = 0,
	MF_Capture     = 1 << 0,
	MF_DoublePush  = 1 << 1,
	MF_EnPassant   = 1 << 2,
	MF_CastleKing  = 1 << 3,
	MF_CastleQueen = 1 << 4,
	MF_Promotion   = 1 << 5
};

struct Move {
	int8_t    From      = NoSquare;
	int8_t    To        = NoSquare;
	uint8_t   Flags     = MF_None;
	PieceType Promotion = PieceType::None;

	bool IsValid() const  { return From >= 0 && To >= 0; }
	bool IsCastle() const { return (Flags & (MF_CastleKing | MF_CastleQueen)) != 0; }
};

inline bool operator==(const Move& _A, const Move& _B)
{
	return _A.From == _B.From && _A.To == _B.To && _A.Promotion == _B.Promotion;
}
inline bool operator!=(const Move& _A, const Move& _B) { return !(_A == _B); }

struct UndoInfo {
	Piece   Captured;
	int8_t  CapturedSquare = NoSquare;
	uint8_t Castling       = CR_None;
	int8_t  EnPassant      = NoSquare;
	int     Halfmove       = 0;
	int     Fullmove       = 1;
};

class Position {

public:

	Position();
	static Position Start();
	static bool FromFen(const std::string& _Fen, Position& _Out);
	std::string ToFen() const;

	Piece At(int _Square) const        { return Squares[_Square]; }
	void  Set(int _Square, Piece _P)   { Squares[_Square] = _P; }

	Color   SideToMove() const  { return Stm; }
	uint8_t Castling() const    { return CastlingRights; }
	int     EnPassant() const   { return EnPassantSquare; }
	int     Halfmove() const    { return HalfmoveClock; }
	int     Fullmove() const    { return FullmoveNumber; }

	int  KingSquare(Color _Side) const;
	bool IsAttacked(int _Square, Color _By) const;
	bool IsInCheck(Color _Side) const;

	int  GeneratePseudoLegal(Move* _OutMoves) const;
	int  GenerateLegal(Move* _OutMoves) const;

	void Make(const Move& _Move, UndoInfo& _Undo);
	void Unmake(const Move& _Move, const UndoInfo& _Undo);

	uint64_t Key() const;
	bool     HasInsufficientMaterial() const;

private:

	void ClearCastlingFor(int _Square);

	Piece   Squares[BoardSquares];
	Color   Stm             = Color::White;
	uint8_t CastlingRights  = CR_All;
	int8_t  EnPassantSquare = NoSquare;
	int     HalfmoveClock   = 0;
	int     FullmoveNumber  = 1;
};

enum class GameResult : uint8_t {
	InProgress,
	WhiteWins,
	BlackWins,
	Stalemate,
	DrawFiftyMove,
	DrawRepetition,
	DrawInsufficientMaterial
};

bool        IsGameOver(GameResult _Result);
std::string ResultText(GameResult _Result);

class Game {

public:

	Game();

	void Reset();
	bool ResetFromFen(const std::string& _Fen);

	const Position& Pos() const   { return Current; }
	GameResult      Result() const { return Outcome; }
	bool            IsOver() const { return IsGameOver(Outcome); }

	int         LegalMoveCount() const { return LegalCount; }
	const Move* LegalMoves() const     { return Legal; }
	int         MovesFrom(int _Square, Move* _OutMoves) const;

	bool Make(const Move& _Move);
	bool Undo();

	bool HasLastMove() const { return !History.empty(); }
	Move LastMove() const;

	int MoveNumber() const { return Current.Fullmove(); }

private:

	void Refresh();
	int  RepetitionCount() const;

	struct HistoryEntry {
		Move     Played;
		UndoInfo Undo;
	};

	Position                  Current;
	std::vector<HistoryEntry> History;
	std::vector<uint64_t>     Keys;
	Move                      Legal[MaxMoves];
	int                       LegalCount = 0;
	GameResult                Outcome    = GameResult::InProgress;
};

uint64_t Perft(Position& _Pos, int _Depth);
bool     RunSelfTest();

}

#endif
