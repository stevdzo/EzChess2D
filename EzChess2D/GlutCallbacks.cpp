#include "GlutCallbacks.h"

#include "BoardLayout.h"
#include "GameWorld.h"
#include "Renderer.h"

namespace {

GameWorld Game;

constexpr int FrameIntervalMs = 16;
constexpr unsigned char KeyEscape = 27;

void DisplayCallback()
{
	glClear(GL_COLOR_BUFFER_BIT);
	Game.Display();
	glutSwapBuffers();
}

void ReshapeCallback(int _Width, int _Height)
{
	Game.Reshape(_Width, _Height);
}

void TimerCallback(int)
{
	glutPostRedisplay();
	glutTimerFunc(FrameIntervalMs, TimerCallback, 0);
}

void MouseCallback(int _Button, int _State, int _X, int _Y)
{
	Game.Mouse(_Button, _State, _X, _Y);
}

void KeyboardCallback(unsigned char _Key, int _X, int _Y)
{
	if (_Key == KeyEscape)
	{
		glutLeaveMainLoop();
		return;
	}

	Game.Keyboard(_Key, _X, _Y);
}

}

int GlutMain(int _Argc, char** _Argv, const std::string& _StartFen)
{
	glutInit(&_Argc, _Argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(120, 60);
	glutInitWindowSize(static_cast<int>(Layout::WorldWidth), static_cast<int>(Layout::WorldHeight));
	glutCreateWindow("EzChess2D");

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	glDisable(GL_DEPTH_TEST);
	glClearColor(0.09f, 0.09f, 0.11f, 1.0f);

	if (!Game.Init(_StartFen))
	{
		return 1;
	}

	glutDisplayFunc(DisplayCallback);
	glutReshapeFunc(ReshapeCallback);
	glutMouseFunc(MouseCallback);
	glutKeyboardFunc(KeyboardCallback);
	glutTimerFunc(FrameIntervalMs, TimerCallback, 0);

	glutMainLoop();

	Game.Shutdown();
	return 0;
}
