#ifndef _INPUT_MANAGER_H
#define _INPUT_MANAGER_H

#include "BoardLayout.h"
#include "Vec2.h"

class InputManager {

public:

	void SetWindowSize(int _Width, int _Height);

	int WindowWidth() const  { return Width; }
	int WindowHeight() const { return Height; }

	Vec2 ToWorld(int _MouseX, int _MouseY) const;

private:

	int Width  = static_cast<int>(Layout::WorldWidth);
	int Height = static_cast<int>(Layout::WorldHeight);
};

#endif
