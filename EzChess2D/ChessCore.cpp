#include "ChessCore.h"

#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>

namespace Chess {

namespace {

struct Delta { int File, Rank; };

const Delta KnightDeltas[8] = {
	{ 1, 2 }, { 2, 1 }, { 2, -1 }, { 1, -2 },
	{ -1, -2 }, { -2, -1 }, { -2, 1 }, { -1, 2 }
};

const Delta KingDeltas[8] = {
	{ 0, 1 }, { 1, 1 }, { 1, 0 }, { 1, -1 },
	{ 0, -1 }, { -1, -1 }, { -1, 0 }, { -1, 1 }
};

const Delta RookDeltas[4]   = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };
const Delta BishopDeltas[4] = { { 1, 1 }, { 1, -1 }, { -1, -1 }, { -1, 1 } };

const PieceType PromotionChoices[4] = {
	PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight
};

inline void AddMove(Move* _Out, int& _Count, int _From, int _To, uint8_t _Flags,
                    PieceType _Promotion = PieceType::None)
{
	if (_Count >= MaxMoves)
	{
		return;
	}

	Move& M     = _Out[_Count++];
	M.From      = static_cast<int8_t>(_From);
	M.To        = static_cast<int8_t>(_To);
	M.Flags     = _Flags;
	M.Promotion = _Promotion;
}

inline void AddPawnMove(Move* _Out, int& _Count, int _From, int _To, uint8_t _Flags, bool _bPromotes)
{
	if (!_bPromotes)
	{
		AddMove(_Out, _Count, _From, _To, _Flags);
		return;
	}

	for (const PieceType Promotion : PromotionChoices)
	{
		AddMove(_Out, _Count, _From, _To, _Flags | MF_Promotion, Promotion);
	}
}

void GeneratePawn(const Position& _Pos, int _From, Move* _Out, int& _Count)
{
	const Color Us        = _Pos.At(_From).Side;
	const int   Dir       = (Us == Color::White) ? 1 : -1;
	const int   StartRank = (Us == Color::White) ? 1 : 6;
	const int   PromoRank = (Us == Color::White) ? 7 : 0;

	const int File = FileOf(_From);
	const int Rank = RankOf(_From);
	const int Next = Rank + Dir;

	if (!IsInside(File, Next))
	{
		return;
	}

	const int OneAhead = SquareOf(File, Next);
	if (_Pos.At(OneAhead).IsEmpty())
	{
		AddPawnMove(_Out, _Count, _From, OneAhead, MF_None, Next == PromoRank);

		if (Rank == StartRank)
		{
			const int TwoAhead = SquareOf(File, Rank + 2 * Dir);
			if (_Pos.At(TwoAhead).IsEmpty())
			{
				AddMove(_Out, _Count, _From, TwoAhead, MF_DoublePush);
			}
		}
	}

	for (int SideStep = -1; SideStep <= 1; SideStep += 2)
	{
		if (!IsInside(File + SideStep, Next))
		{
			continue;
		}

		const int   To     = SquareOf(File + SideStep, Next);
		const Piece Target = _Pos.At(To);

		if (Target.Is(Opposite(Us)))
		{
			AddPawnMove(_Out, _Count, _From, To, MF_Capture, Next == PromoRank);
		}
		else if (Target.IsEmpty() && To == _Pos.EnPassant())
		{
			AddMove(_Out, _Count, _From, To, MF_Capture | MF_EnPassant);
		}
	}
}

void GenerateSteps(const Position& _Pos, int _From, const Delta* _Deltas, int _DeltaCount,
                   Move* _Out, int& _Count)
{
	const Color Us   = _Pos.At(_From).Side;
	const int   File = FileOf(_From);
	const int   Rank = RankOf(_From);

	for (int Index = 0; Index < _DeltaCount; ++Index)
	{
		const int ToFile = File + _Deltas[Index].File;
		const int ToRank = Rank + _Deltas[Index].Rank;

		if (!IsInside(ToFile, ToRank))
		{
			continue;
		}

		const int   To     = SquareOf(ToFile, ToRank);
		const Piece Target = _Pos.At(To);

		if (Target.Is(Us))
		{
			continue;
		}

		AddMove(_Out, _Count, _From, To, Target.IsEmpty() ? MF_None : MF_Capture);
	}
}

void GenerateSlides(const Position& _Pos, int _From, const Delta* _Deltas, int _DeltaCount,
                    Move* _Out, int& _Count)
{
	const Color Us   = _Pos.At(_From).Side;
	const int   File = FileOf(_From);
	const int   Rank = RankOf(_From);

	for (int Index = 0; Index < _DeltaCount; ++Index)
	{
		int ToFile = File + _Deltas[Index].File;
		int ToRank = Rank + _Deltas[Index].Rank;

		while (IsInside(ToFile, ToRank))
		{
			const int   To     = SquareOf(ToFile, ToRank);
			const Piece Target = _Pos.At(To);

			if (Target.Is(Us))
			{
				break;
			}

			AddMove(_Out, _Count, _From, To, Target.IsEmpty() ? MF_None : MF_Capture);

			if (!Target.IsEmpty())
			{
				break;
			}

			ToFile += _Deltas[Index].File;
			ToRank += _Deltas[Index].Rank;
		}
	}
}

void GenerateCastles(const Position& _Pos, Move* _Out, int& _Count)
{
	const Color Us      = _Pos.SideToMove();
	const Color Them    = Opposite(Us);
	const int   Rank    = (Us == Color::White) ? 0 : 7;
	const int   KingSq  = SquareOf(4, Rank);

	const Piece King = _Pos.At(KingSq);
	if (King.Type != PieceType::King || King.Side != Us)
	{
		return;
	}

	if (_Pos.IsAttacked(KingSq, Them))
	{
		return;
	}

	const uint8_t KingSideRight  = (Us == Color::White) ? CR_WhiteKing : CR_BlackKing;
	const uint8_t QueenSideRight = (Us == Color::White) ? CR_WhiteQueen : CR_BlackQueen;

	if (_Pos.Castling() & KingSideRight)
	{
		const Piece Rook = _Pos.At(SquareOf(7, Rank));
		const bool  bPathClear = _Pos.At(SquareOf(5, Rank)).IsEmpty() && _Pos.At(SquareOf(6, Rank)).IsEmpty();

		if (Rook.Type == PieceType::Rook && Rook.Side == Us && bPathClear &&
			!_Pos.IsAttacked(SquareOf(5, Rank), Them))
		{
			AddMove(_Out, _Count, KingSq, SquareOf(6, Rank), MF_CastleKing);
		}
	}

	if (_Pos.Castling() & QueenSideRight)
	{
		const Piece Rook = _Pos.At(SquareOf(0, Rank));
		const bool  bPathClear = _Pos.At(SquareOf(1, Rank)).IsEmpty() &&
		                         _Pos.At(SquareOf(2, Rank)).IsEmpty() &&
		                         _Pos.At(SquareOf(3, Rank)).IsEmpty();

		if (Rook.Type == PieceType::Rook && Rook.Side == Us && bPathClear &&
			!_Pos.IsAttacked(SquareOf(3, Rank), Them))
		{
			AddMove(_Out, _Count, KingSq, SquareOf(2, Rank), MF_CastleQueen);
		}
	}
}

char PieceToFenChar(Piece _Piece)
{
	char Symbol = '?';
	switch (_Piece.Type)
	{
	case PieceType::Pawn:   Symbol = 'p'; break;
	case PieceType::Knight: Symbol = 'n'; break;
	case PieceType::Bishop: Symbol = 'b'; break;
	case PieceType::Rook:   Symbol = 'r'; break;
	case PieceType::Queen:  Symbol = 'q'; break;
	case PieceType::King:   Symbol = 'k'; break;
	default:                return '?';
	}
	return (_Piece.Side == Color::White) ? static_cast<char>(Symbol - 'a' + 'A') : Symbol;
}

bool FenCharToPiece(char _Symbol, Piece& _Out)
{
	const bool bIsWhite = (_Symbol >= 'A' && _Symbol <= 'Z');
	const char Lower    = bIsWhite ? static_cast<char>(_Symbol - 'A' + 'a') : _Symbol;

	switch (Lower)
	{
	case 'p': _Out.Type = PieceType::Pawn;   break;
	case 'n': _Out.Type = PieceType::Knight; break;
	case 'b': _Out.Type = PieceType::Bishop; break;
	case 'r': _Out.Type = PieceType::Rook;   break;
	case 'q': _Out.Type = PieceType::Queen;  break;
	case 'k': _Out.Type = PieceType::King;   break;
	default:  return false;
	}

	_Out.Side = bIsWhite ? Color::White : Color::Black;
	return true;
}

}

std::string SquareName(int _Square)
{
	if (!IsSquare(_Square))
	{
		return "-";
	}

	std::string Name;
	Name += static_cast<char>('a' + FileOf(_Square));
	Name += static_cast<char>('1' + RankOf(_Square));
	return Name;
}

int SquareFromName(const std::string& _Name)
{
	if (_Name.size() < 2 || _Name[0] < 'a' || _Name[0] > 'h' || _Name[1] < '1' || _Name[1] > '8')
	{
		return NoSquare;
	}
	return SquareOf(_Name[0] - 'a', _Name[1] - '1');
}

Position::Position()
{
	CastlingRights = CR_None;
}

Position Position::Start()
{
	Position Result;
	FromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", Result);
	return Result;
}

bool Position::FromFen(const std::string& _Fen, Position& _Out)
{
	Position Parsed;

	std::istringstream Stream(_Fen);
	std::string Placement, SideToMoveField, CastlingField, EnPassantField;

	if (!(Stream >> Placement >> SideToMoveField >> CastlingField >> EnPassantField))
	{
		return false;
	}

	int File = 0;
	int Rank = 7;
	for (const char Symbol : Placement)
	{
		if (Symbol == '/')
		{
			--Rank;
			File = 0;
			continue;
		}

		if (Symbol >= '1' && Symbol <= '8')
		{
			File += Symbol - '0';
			continue;
		}

		Piece Parsed_Piece;
		if (!FenCharToPiece(Symbol, Parsed_Piece) || !IsInside(File, Rank))
		{
			return false;
		}

		Parsed.Squares[SquareOf(File, Rank)] = Parsed_Piece;
		++File;
	}

	Parsed.Stm = (SideToMoveField == "b") ? Color::Black : Color::White;

	Parsed.CastlingRights = CR_None;
	for (const char Symbol : CastlingField)
	{
		switch (Symbol)
		{
		case 'K': Parsed.CastlingRights |= CR_WhiteKing;  break;
		case 'Q': Parsed.CastlingRights |= CR_WhiteQueen; break;
		case 'k': Parsed.CastlingRights |= CR_BlackKing;  break;
		case 'q': Parsed.CastlingRights |= CR_BlackQueen; break;
		default: break;
		}
	}

	Parsed.EnPassantSquare = static_cast<int8_t>(
		(EnPassantField == "-") ? NoSquare : SquareFromName(EnPassantField));

	int Halfmove = 0;
	int Fullmove = 1;
	if (Stream >> Halfmove)
	{
		Parsed.HalfmoveClock = Halfmove;
	}
	if (Stream >> Fullmove)
	{
		Parsed.FullmoveNumber = Fullmove;
	}

	_Out = Parsed;
	return true;
}

std::string Position::ToFen() const
{
	std::string Fen;

	for (int Rank = 7; Rank >= 0; --Rank)
	{
		int EmptyRun = 0;
		for (int File = 0; File < 8; ++File)
		{
			const Piece Current = Squares[SquareOf(File, Rank)];
			if (Current.IsEmpty())
			{
				++EmptyRun;
				continue;
			}

			if (EmptyRun > 0)
			{
				Fen += static_cast<char>('0' + EmptyRun);
				EmptyRun = 0;
			}
			Fen += PieceToFenChar(Current);
		}

		if (EmptyRun > 0)
		{
			Fen += static_cast<char>('0' + EmptyRun);
		}
		if (Rank > 0)
		{
			Fen += '/';
		}
	}

	Fen += (Stm == Color::White) ? " w " : " b ";

	if (CastlingRights == CR_None)
	{
		Fen += '-';
	}
	else
	{
		if (CastlingRights & CR_WhiteKing)  Fen += 'K';
		if (CastlingRights & CR_WhiteQueen) Fen += 'Q';
		if (CastlingRights & CR_BlackKing)  Fen += 'k';
		if (CastlingRights & CR_BlackQueen) Fen += 'q';
	}

	Fen += ' ';
	Fen += (EnPassantSquare == NoSquare) ? "-" : SquareName(EnPassantSquare);

	Fen += ' ' + std::to_string(HalfmoveClock);
	Fen += ' ' + std::to_string(FullmoveNumber);
	return Fen;
}

int Position::KingSquare(Color _Side) const
{
	for (int Square = 0; Square < BoardSquares; ++Square)
	{
		if (Squares[Square].Type == PieceType::King && Squares[Square].Side == _Side)
		{
			return Square;
		}
	}
	return NoSquare;
}

bool Position::IsAttacked(int _Square, Color _By) const
{
	const int File = FileOf(_Square);
	const int Rank = RankOf(_Square);

	const int PawnRank = (_By == Color::White) ? Rank - 1 : Rank + 1;
	for (int SideStep = -1; SideStep <= 1; SideStep += 2)
	{
		if (!IsInside(File + SideStep, PawnRank))
		{
			continue;
		}

		const Piece Current = Squares[SquareOf(File + SideStep, PawnRank)];
		if (Current.Type == PieceType::Pawn && Current.Side == _By)
		{
			return true;
		}
	}

	for (const Delta& Step : KnightDeltas)
	{
		if (!IsInside(File + Step.File, Rank + Step.Rank))
		{
			continue;
		}

		const Piece Current = Squares[SquareOf(File + Step.File, Rank + Step.Rank)];
		if (Current.Type == PieceType::Knight && Current.Side == _By)
		{
			return true;
		}
	}

	for (const Delta& Step : KingDeltas)
	{
		if (!IsInside(File + Step.File, Rank + Step.Rank))
		{
			continue;
		}

		const Piece Current = Squares[SquareOf(File + Step.File, Rank + Step.Rank)];
		if (Current.Type == PieceType::King && Current.Side == _By)
		{
			return true;
		}
	}

	for (const Delta& Step : RookDeltas)
	{
		int ToFile = File + Step.File;
		int ToRank = Rank + Step.Rank;

		while (IsInside(ToFile, ToRank))
		{
			const Piece Current = Squares[SquareOf(ToFile, ToRank)];
			if (!Current.IsEmpty())
			{
				if (Current.Side == _By &&
					(Current.Type == PieceType::Rook || Current.Type == PieceType::Queen))
				{
					return true;
				}
				break;
			}

			ToFile += Step.File;
			ToRank += Step.Rank;
		}
	}

	for (const Delta& Step : BishopDeltas)
	{
		int ToFile = File + Step.File;
		int ToRank = Rank + Step.Rank;

		while (IsInside(ToFile, ToRank))
		{
			const Piece Current = Squares[SquareOf(ToFile, ToRank)];
			if (!Current.IsEmpty())
			{
				if (Current.Side == _By &&
					(Current.Type == PieceType::Bishop || Current.Type == PieceType::Queen))
				{
					return true;
				}
				break;
			}

			ToFile += Step.File;
			ToRank += Step.Rank;
		}
	}

	return false;
}

bool Position::IsInCheck(Color _Side) const
{
	const int King = KingSquare(_Side);
	return King != NoSquare && IsAttacked(King, Opposite(_Side));
}

int Position::GeneratePseudoLegal(Move* _OutMoves) const
{
	int Count = 0;

	for (int Square = 0; Square < BoardSquares; ++Square)
	{
		const Piece Current = Squares[Square];
		if (!Current.Is(Stm))
		{
			continue;
		}

		switch (Current.Type)
		{
		case PieceType::Pawn:
			GeneratePawn(*this, Square, _OutMoves, Count);
			break;
		case PieceType::Knight:
			GenerateSteps(*this, Square, KnightDeltas, 8, _OutMoves, Count);
			break;
		case PieceType::Bishop:
			GenerateSlides(*this, Square, BishopDeltas, 4, _OutMoves, Count);
			break;
		case PieceType::Rook:
			GenerateSlides(*this, Square, RookDeltas, 4, _OutMoves, Count);
			break;
		case PieceType::Queen:
			GenerateSlides(*this, Square, RookDeltas, 4, _OutMoves, Count);
			GenerateSlides(*this, Square, BishopDeltas, 4, _OutMoves, Count);
			break;
		case PieceType::King:
			GenerateSteps(*this, Square, KingDeltas, 8, _OutMoves, Count);
			break;
		default:
			break;
		}
	}

	GenerateCastles(*this, _OutMoves, Count);
	return Count;
}

int Position::GenerateLegal(Move* _OutMoves) const
{
	Move Pseudo[MaxMoves];
	const int PseudoCount = GeneratePseudoLegal(Pseudo);

	Position Working = *this;
	const Color Us   = Stm;

	int Count = 0;
	for (int Index = 0; Index < PseudoCount; ++Index)
	{
		UndoInfo Undo;
		Working.Make(Pseudo[Index], Undo);

		if (!Working.IsInCheck(Us))
		{
			_OutMoves[Count++] = Pseudo[Index];
		}

		Working.Unmake(Pseudo[Index], Undo);
	}

	return Count;
}

void Position::ClearCastlingFor(int _Square)
{
	switch (_Square)
	{
	case 0:  CastlingRights = static_cast<uint8_t>(CastlingRights & ~CR_WhiteQueen); break;  // a1
	case 7:  CastlingRights = static_cast<uint8_t>(CastlingRights & ~CR_WhiteKing);  break;  // h1
	case 56: CastlingRights = static_cast<uint8_t>(CastlingRights & ~CR_BlackQueen); break;  // a8
	case 63: CastlingRights = static_cast<uint8_t>(CastlingRights & ~CR_BlackKing);  break;  // h8
	default: break;
	}
}

void Position::Make(const Move& _Move, UndoInfo& _Undo)
{
	_Undo.Captured       = Piece();
	_Undo.CapturedSquare = NoSquare;
	_Undo.Castling       = CastlingRights;
	_Undo.EnPassant      = EnPassantSquare;
	_Undo.Halfmove       = HalfmoveClock;
	_Undo.Fullmove       = FullmoveNumber;

	const Piece Mover = Squares[_Move.From];
	const Color Us    = Mover.Side;

	int CaptureSquare = NoSquare;
	if (_Move.Flags & MF_EnPassant)
	{
		CaptureSquare = SquareOf(FileOf(_Move.To), RankOf(_Move.From));
	}
	else if (!Squares[_Move.To].IsEmpty())
	{
		CaptureSquare = _Move.To;
	}

	if (CaptureSquare != NoSquare)
	{
		_Undo.Captured       = Squares[CaptureSquare];
		_Undo.CapturedSquare = static_cast<int8_t>(CaptureSquare);
		Squares[CaptureSquare] = Piece();
	}

	Squares[_Move.To]   = Mover;
	Squares[_Move.From] = Piece();

	if (_Move.Flags & MF_Promotion)
	{
		Squares[_Move.To].Type = _Move.Promotion;
	}

	if (_Move.Flags & MF_CastleKing)
	{
		const int Rank = RankOf(_Move.From);
		Squares[SquareOf(5, Rank)] = Squares[SquareOf(7, Rank)];
		Squares[SquareOf(7, Rank)] = Piece();
	}
	else if (_Move.Flags & MF_CastleQueen)
	{
		const int Rank = RankOf(_Move.From);
		Squares[SquareOf(3, Rank)] = Squares[SquareOf(0, Rank)];
		Squares[SquareOf(0, Rank)] = Piece();
	}

	EnPassantSquare = NoSquare;
	if (_Move.Flags & MF_DoublePush)
	{
		EnPassantSquare = static_cast<int8_t>(
			SquareOf(FileOf(_Move.From), (RankOf(_Move.From) + RankOf(_Move.To)) / 2));
	}

	if (Mover.Type == PieceType::King)
	{
		const uint8_t Mask = (Us == Color::White) ? (CR_WhiteKing | CR_WhiteQueen)
		                                          : (CR_BlackKing | CR_BlackQueen);
		CastlingRights = static_cast<uint8_t>(CastlingRights & ~Mask);
	}

	ClearCastlingFor(_Move.From);
	ClearCastlingFor(_Move.To);

	if (Mover.Type == PieceType::Pawn || CaptureSquare != NoSquare)
	{
		HalfmoveClock = 0;
	}
	else
	{
		++HalfmoveClock;
	}

	if (Us == Color::Black)
	{
		++FullmoveNumber;
	}

	Stm = Opposite(Us);
}

void Position::Unmake(const Move& _Move, const UndoInfo& _Undo)
{
	Stm = Opposite(Stm);

	Piece Mover = Squares[_Move.To];
	Squares[_Move.To] = Piece();

	if (_Move.Flags & MF_Promotion)
	{
		Mover.Type = PieceType::Pawn;
	}
	Squares[_Move.From] = Mover;

	if (_Move.Flags & MF_CastleKing)
	{
		const int Rank = RankOf(_Move.From);
		Squares[SquareOf(7, Rank)] = Squares[SquareOf(5, Rank)];
		Squares[SquareOf(5, Rank)] = Piece();
	}
	else if (_Move.Flags & MF_CastleQueen)
	{
		const int Rank = RankOf(_Move.From);
		Squares[SquareOf(0, Rank)] = Squares[SquareOf(3, Rank)];
		Squares[SquareOf(3, Rank)] = Piece();
	}

	if (_Undo.CapturedSquare != NoSquare)
	{
		Squares[_Undo.CapturedSquare] = _Undo.Captured;
	}

	CastlingRights  = _Undo.Castling;
	EnPassantSquare = _Undo.EnPassant;
	HalfmoveClock   = _Undo.Halfmove;
	FullmoveNumber  = _Undo.Fullmove;
}

uint64_t Position::Key() const
{
	uint64_t Hash = 1469598103934665603ULL;

	const auto Mix = [&Hash](uint8_t _Byte)
	{
		Hash ^= _Byte;
		Hash *= 1099511628211ULL;
	};

	for (int Square = 0; Square < BoardSquares; ++Square)
	{
		Mix(static_cast<uint8_t>(Squares[Square].Type));
		Mix(static_cast<uint8_t>(Squares[Square].Side));
	}

	Mix(static_cast<uint8_t>(Stm));
	Mix(CastlingRights);
	Mix(static_cast<uint8_t>(EnPassantSquare + 1));
	return Hash;
}

bool Position::HasInsufficientMaterial() const
{
	int Minors[2]      = { 0, 0 };
	int Bishops[2]     = { 0, 0 };
	int BishopColor[2] = { -1, -1 };

	for (int Square = 0; Square < BoardSquares; ++Square)
	{
		const Piece Current = Squares[Square];
		if (Current.IsEmpty())
		{
			continue;
		}

		const int Index = (Current.Side == Color::White) ? 0 : 1;

		switch (Current.Type)
		{
		case PieceType::King:
			break;
		case PieceType::Knight:
			++Minors[Index];
			break;
		case PieceType::Bishop:
			++Minors[Index];
			++Bishops[Index];
			BishopColor[Index] = (FileOf(Square) + RankOf(Square)) & 1;
			break;
		default:
			return false;
		}
	}

	if (Minors[0] <= 1 && Minors[1] == 0)
	{
		return true;
	}
	if (Minors[1] <= 1 && Minors[0] == 0)
	{
		return true;
	}
	if (Bishops[0] == 1 && Bishops[1] == 1 && Minors[0] == 1 && Minors[1] == 1 &&
		BishopColor[0] == BishopColor[1])
	{
		return true;
	}

	return false;
}

bool IsGameOver(GameResult _Result)
{
	return _Result != GameResult::InProgress;
}

std::string ResultText(GameResult _Result)
{
	switch (_Result)
	{
	case GameResult::WhiteWins:                return "MAT - beli pobedjuje";
	case GameResult::BlackWins:                return "MAT - crni pobedjuje";
	case GameResult::Stalemate:                return "PAT - remi";
	case GameResult::DrawFiftyMove:            return "REMI - pravilo 50 poteza";
	case GameResult::DrawRepetition:           return "REMI - trostruko ponavljanje";
	case GameResult::DrawInsufficientMaterial: return "REMI - nedovoljan materijal";
	default:                                   return "";
	}
}

Game::Game()
{
	Reset();
}

void Game::Reset()
{
	Current = Position::Start();
	History.clear();
	Keys.clear();
	Keys.push_back(Current.Key());
	Refresh();
}

bool Game::ResetFromFen(const std::string& _Fen)
{
	Position Parsed;
	if (!Position::FromFen(_Fen, Parsed))
	{
		return false;
	}

	Current = Parsed;
	History.clear();
	Keys.clear();
	Keys.push_back(Current.Key());
	Refresh();
	return true;
}

int Game::MovesFrom(int _Square, Move* _OutMoves) const
{
	int Count = 0;
	for (int Index = 0; Index < LegalCount; ++Index)
	{
		if (Legal[Index].From == _Square)
		{
			_OutMoves[Count++] = Legal[Index];
		}
	}
	return Count;
}

bool Game::Make(const Move& _Move)
{
	if (IsOver())
	{
		return false;
	}

	for (int Index = 0; Index < LegalCount; ++Index)
	{
		if (Legal[Index] != _Move)
		{
			continue;
		}

		HistoryEntry Entry;
		Entry.Played = Legal[Index];
		Current.Make(Entry.Played, Entry.Undo);

		History.push_back(Entry);
		Keys.push_back(Current.Key());
		Refresh();
		return true;
	}

	return false;
}

bool Game::Undo()
{
	if (History.empty())
	{
		return false;
	}

	const HistoryEntry Entry = History.back();
	History.pop_back();
	Keys.pop_back();

	Current.Unmake(Entry.Played, Entry.Undo);
	Refresh();
	return true;
}

Move Game::LastMove() const
{
	return History.empty() ? Move() : History.back().Played;
}

int Game::RepetitionCount() const
{
	if (Keys.empty())
	{
		return 0;
	}

	const uint64_t CurrentKey = Keys.back();

	int Count = 0;
	for (const uint64_t Key : Keys)
	{
		if (Key == CurrentKey)
		{
			++Count;
		}
	}
	return Count;
}

void Game::Refresh()
{
	LegalCount = Current.GenerateLegal(Legal);

	if (LegalCount == 0)
	{
		if (Current.IsInCheck(Current.SideToMove()))
		{
			Outcome = (Current.SideToMove() == Color::White) ? GameResult::BlackWins
			                                                 : GameResult::WhiteWins;
		}
		else
		{
			Outcome = GameResult::Stalemate;
		}
		return;
	}

	if (Current.Halfmove() >= 100)
	{
		Outcome = GameResult::DrawFiftyMove;
		return;
	}

	if (RepetitionCount() >= 3)
	{
		Outcome = GameResult::DrawRepetition;
		return;
	}

	if (Current.HasInsufficientMaterial())
	{
		Outcome = GameResult::DrawInsufficientMaterial;
		return;
	}

	Outcome = GameResult::InProgress;
}

uint64_t Perft(Position& _Pos, int _Depth)
{
	if (_Depth == 0)
	{
		return 1;
	}

	Move Moves[MaxMoves];
	const int Count = _Pos.GenerateLegal(Moves);

	if (_Depth == 1)
	{
		return static_cast<uint64_t>(Count);
	}

	uint64_t Nodes = 0;
	for (int Index = 0; Index < Count; ++Index)
	{
		UndoInfo Undo;
		_Pos.Make(Moves[Index], Undo);
		Nodes += Perft(_Pos, _Depth - 1);
		_Pos.Unmake(Moves[Index], Undo);
	}

	return Nodes;
}

bool RunSelfTest()
{
	struct TestCase {
		const char* Name;
		const char* Fen;
		int         Depth;
		uint64_t    Expected;
	};

	const TestCase Cases[] = {
		{ "Start position",   "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 1, 20ULL },
		{ "Start position",   "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 2, 400ULL },
		{ "Start position",   "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 3, 8902ULL },
		{ "Start position",   "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 4, 197281ULL },
		{ "Kiwipete",         "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 3, 97862ULL },
		{ "Kiwipete",         "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 4, 4085603ULL },
		{ "Position 3",       "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 4, 43238ULL },
		{ "Position 3",       "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 5, 674624ULL },
		{ "Position 4",       "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w KQkq - 0 1", 4, 422333ULL },
		{ "Position 5",       "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 4, 2103487ULL },
		{ "Position 6",       "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 4, 3894594ULL }
	};

	bool bAllPassed = true;

	for (const TestCase& Case : Cases)
	{
		Position Pos;
		if (!Position::FromFen(Case.Fen, Pos))
		{
			std::cout << "[PERFT] FEN se ne parsira: " << Case.Fen << std::endl;
			bAllPassed = false;
			continue;
		}

		const uint64_t Nodes  = Perft(Pos, Case.Depth);
		const bool     bMatch = (Nodes == Case.Expected);

		std::cout << (bMatch ? "[ OK ] " : "[FAIL] ")
		          << Case.Name << "  depth " << Case.Depth
		          << "  expected " << Case.Expected
		          << "  actual   " << Nodes << std::endl;

		bAllPassed = bAllPassed && bMatch;
	}

	std::cout << (bAllPassed ? "[PERFT] All cases pass." : "[PERFT] HAS ERRORS.") << std::endl;
	return bAllPassed;
}

}
