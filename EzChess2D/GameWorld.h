#ifndef _GAME_WORLD_H
#define _GAME_WORLD_H

#include <string>

#include "GameManager.h"
#include "InputManager.h"
#include "SceneManager.h"

class GameWorld {

public:

	bool Init(const std::string& _StartFen);
	void Shutdown();

	void Display() const;
	void Reshape(int _Width, int _Height);

	void Mouse(int _Button, int _State, int _X, int _Y);
	void Keyboard(unsigned char _Key, int _X, int _Y);

private:

	SceneManager Scene;
	InputManager Input;
	GameManager  Manager;
};

#endif
