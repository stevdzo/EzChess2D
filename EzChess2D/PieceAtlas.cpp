#include "PieceAtlas.h"

#ifdef __APPLE__
#include <GLUT/freeglut.h>
#else
#include <GL/freeglut.h>
#endif

#include "SOIL2/SOIL2.h"

namespace PieceAtlas {

namespace {

int  SharedTexture = 0;
Vec2 AtlasFrames(6.0f, 2.0f);

}

bool Load(const std::string& _FilePath)
{
	if (SharedTexture != 0)
	{
		return true;
	}

	SharedTexture = SOIL_load_OGL_texture(_FilePath.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
	return SharedTexture != 0;
}

void Release()
{
	if (SharedTexture == 0)
	{
		return;
	}

	const GLuint TextureId = static_cast<GLuint>(SharedTexture);
	glDeleteTextures(1, &TextureId);
	SharedTexture = 0;
}

bool IsLoaded()
{
	return SharedTexture != 0;
}

int Texture()
{
	return SharedTexture;
}

Vec2 Frames()
{
	return AtlasFrames;
}

int IndexFor(Chess::PieceType _Type, Chess::Color _Side)
{
	int Offset = 0;
	switch (_Type)
	{
	case Chess::PieceType::King:   Offset = 0; break;
	case Chess::PieceType::Pawn:   Offset = 1; break;
	case Chess::PieceType::Bishop: Offset = 2; break;
	case Chess::PieceType::Queen:  Offset = 3; break;
	case Chess::PieceType::Rook:   Offset = 4; break;
	case Chess::PieceType::Knight: Offset = 5; break;
	default:                       return -1;
	}

	return (_Side == Chess::Color::White ? 6 : 0) + Offset;
}

}
