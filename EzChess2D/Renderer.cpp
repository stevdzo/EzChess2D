#include "Renderer.h"

#include <cmath>

namespace {

constexpr float Pi          = 3.14159265358979323846f;
constexpr int   CircleSteps = 48;

void BeginBlend()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void EndBlend()
{
	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

}

void Renderer::RenderSprite(int _Texture, int _Index, Vec2 _Frames, Vec2 _Position, Vec2 _Size) const
{
	if (_Texture == 0 || _Index < 0 || _Frames.X <= 0.0f || _Frames.Y <= 0.0f)
	{
		return;
	}

	BeginBlend();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(_Texture));

	const float StepX = 1.0f / _Frames.X;
	const float StepY = 1.0f / _Frames.Y;

	const int IndexX = _Index % static_cast<int>(_Frames.X);
	const int IndexY = _Index / static_cast<int>(_Frames.X);

	const float TexX = IndexX * StepX;
	const float TexY = IndexY * StepY;

	const float HalfX = _Size.X * 0.5f;
	const float HalfY = _Size.Y * 0.5f;

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
		glTexCoord2f(TexX,         TexY + StepY); glVertex2f(_Position.X - HalfX, _Position.Y - HalfY);
		glTexCoord2f(TexX + StepX, TexY + StepY); glVertex2f(_Position.X + HalfX, _Position.Y - HalfY);
		glTexCoord2f(TexX + StepX, TexY);         glVertex2f(_Position.X + HalfX, _Position.Y + HalfY);
		glTexCoord2f(TexX,         TexY);         glVertex2f(_Position.X - HalfX, _Position.Y + HalfY);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	EndBlend();
}

void Renderer::RenderFillRect(Vec2 _Position, Vec2 _Size, Vec3 _Color, float _Alpha) const
{
	const float HalfX = _Size.X * 0.5f;
	const float HalfY = _Size.Y * 0.5f;

	BeginBlend();
	glColor4f(_Color.R, _Color.G, _Color.B, _Alpha);
	glBegin(GL_QUADS);
		glVertex2f(_Position.X - HalfX, _Position.Y - HalfY);
		glVertex2f(_Position.X + HalfX, _Position.Y - HalfY);
		glVertex2f(_Position.X + HalfX, _Position.Y + HalfY);
		glVertex2f(_Position.X - HalfX, _Position.Y + HalfY);
	glEnd();
	EndBlend();
}

void Renderer::RenderWireframeRect(Vec2 _Position, Vec2 _Size, Vec3 _Color, float _LineWidth) const
{
	const float HalfX = _Size.X * 0.5f;
	const float HalfY = _Size.Y * 0.5f;
	const float Inset = _LineWidth * 0.5f;

	BeginBlend();
	glColor4f(_Color.R, _Color.G, _Color.B, 1.0f);
	glLineWidth(_LineWidth);
	glBegin(GL_LINE_LOOP);
		glVertex2f(_Position.X - HalfX + Inset, _Position.Y - HalfY + Inset);
		glVertex2f(_Position.X + HalfX - Inset, _Position.Y - HalfY + Inset);
		glVertex2f(_Position.X + HalfX - Inset, _Position.Y + HalfY - Inset);
		glVertex2f(_Position.X - HalfX + Inset, _Position.Y + HalfY - Inset);
	glEnd();
	glLineWidth(1.0f);
	EndBlend();
}

void Renderer::RenderFillCircle(Vec2 _Center, float _Radius, Vec3 _Color, float _Alpha) const
{
	BeginBlend();
	glColor4f(_Color.R, _Color.G, _Color.B, _Alpha);
	glBegin(GL_TRIANGLE_FAN);
		glVertex2f(_Center.X, _Center.Y);
		for (int Step = 0; Step <= CircleSteps; ++Step)
		{
			const float Theta = 2.0f * Pi * static_cast<float>(Step) / static_cast<float>(CircleSteps);
			glVertex2f(_Center.X + _Radius * std::cos(Theta), _Center.Y + _Radius * std::sin(Theta));
		}
	glEnd();
	EndBlend();
}

void Renderer::RenderRing(Vec2 _Center, float _Radius, float _Thickness, Vec3 _Color, float _Alpha) const
{
	const float Inner = _Radius - _Thickness;

	BeginBlend();
	glColor4f(_Color.R, _Color.G, _Color.B, _Alpha);
	glBegin(GL_TRIANGLE_STRIP);
		for (int Step = 0; Step <= CircleSteps; ++Step)
		{
			const float Theta = 2.0f * Pi * static_cast<float>(Step) / static_cast<float>(CircleSteps);
			const float CosT  = std::cos(Theta);
			const float SinT  = std::sin(Theta);

			glVertex2f(_Center.X + Inner * CosT, _Center.Y + Inner * SinT);
			glVertex2f(_Center.X + _Radius * CosT, _Center.Y + _Radius * SinT);
		}
	glEnd();
	EndBlend();
}

void Renderer::RenderText(Vec2 _Position, const std::string& _Text, Vec3 _Color, void* _Font) const
{
	glColor4f(_Color.R, _Color.G, _Color.B, 1.0f);
	glRasterPos2f(_Position.X, _Position.Y);

	for (const char Symbol : _Text)
	{
		glutBitmapCharacter(_Font, Symbol);
	}

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void Renderer::RenderTextCentered(Vec2 _Center, const std::string& _Text, Vec3 _Color, void* _Font) const
{
	RenderText(Vec2(_Center.X - MeasureText(_Text, _Font) * 0.5f, _Center.Y), _Text, _Color, _Font);
}

float Renderer::MeasureText(const std::string& _Text, void* _Font) const
{
	return static_cast<float>(
		glutBitmapLength(_Font, reinterpret_cast<const unsigned char*>(_Text.c_str())));
}
