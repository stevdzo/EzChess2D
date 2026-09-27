#ifndef _RENDERER_H
#define _RENDERER_H

#ifdef __APPLE__
#include <GLUT/freeglut.h>
#else
#include <GL/freeglut.h>
#endif

#include <string>

#include "Vec2.h"
#include "Vec3.h"

class Renderer {

public:

	void RenderSprite(int _Texture, int _Index, Vec2 _Frames, Vec2 _Position, Vec2 _Size) const;

	void RenderFillRect(Vec2 _Position, Vec2 _Size, Vec3 _Color, float _Alpha = 1.0f) const;
	void RenderWireframeRect(Vec2 _Position, Vec2 _Size, Vec3 _Color, float _LineWidth = 3.0f) const;

	void RenderFillCircle(Vec2 _Center, float _Radius, Vec3 _Color, float _Alpha = 1.0f) const;
	void RenderRing(Vec2 _Center, float _Radius, float _Thickness, Vec3 _Color, float _Alpha = 1.0f) const;

	void RenderText(Vec2 _Position, const std::string& _Text, Vec3 _Color, void* _Font = GLUT_BITMAP_HELVETICA_18) const;
	void RenderTextCentered(Vec2 _Center, const std::string& _Text, Vec3 _Color, void* _Font = GLUT_BITMAP_HELVETICA_18) const;

	float MeasureText(const std::string& _Text, void* _Font = GLUT_BITMAP_HELVETICA_18) const;
};

#endif
