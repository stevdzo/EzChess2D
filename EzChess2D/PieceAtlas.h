#ifndef _PIECE_ATLAS_H
#define _PIECE_ATLAS_H

#include <string>

#include "ChessCore.h"
#include "Vec2.h"

namespace PieceAtlas {

bool Load(const std::string& _FilePath);
void Release();

bool IsLoaded();
int  Texture();
Vec2 Frames();

int IndexFor(Chess::PieceType _Type, Chess::Color _Side);

}

#endif
