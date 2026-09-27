#include "GameWorld.h"

#include "BoardLayout.h"
#include "DebugUtils.h"
#include "PieceAtlas.h"
#include "Renderer.h"

bool GameWorld::Init(const std::string& _StartFen)
{
	if (!PieceAtlas::Load("resources/pieces.png"))
	{
		Print("GameWorld::Init -- ne mogu da ucitam resources/pieces.png");
		return false;
	}

	Manager.SetStartFen(_StartFen);
	return true;
}

void GameWorld::Shutdown()
{
	PieceAtlas::Release();
}

void GameWorld::Display() const
{
	Scene.Display(Manager);
}

void GameWorld::Reshape(int _Width, int _Height)
{
	Input.SetWindowSize(_Width, _Height);

	const Layout::ViewportRect View = Layout::Letterbox(_Width, _Height);
	glViewport(View.X, View.Y, View.Width, View.Height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, Layout::WorldWidth, 0.0, Layout::WorldHeight);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void GameWorld::Mouse(int _Button, int _State, int _X, int _Y)
{
	if (_Button != GLUT_LEFT_BUTTON || _State != GLUT_DOWN)
	{
		return;
	}

	Manager.OnClick(Input.ToWorld(_X, _Y));
}

void GameWorld::Keyboard(unsigned char _Key, int /*_X*/, int /*_Y*/)
{
	Manager.OnKey(_Key);
}
