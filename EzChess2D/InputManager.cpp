#include "InputManager.h"

void InputManager::SetWindowSize(int _Width, int _Height)
{
	Width  = (_Width > 0) ? _Width : 1;
	Height = (_Height > 0) ? _Height : 1;
}

Vec2 InputManager::ToWorld(int _MouseX, int _MouseY) const
{
	return Layout::MouseToWorld(_MouseX, _MouseY, Width, Height);
}
