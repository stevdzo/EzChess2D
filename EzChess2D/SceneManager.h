#ifndef _SCENE_MANAGER_H
#define _SCENE_MANAGER_H

#include "GameManager.h"
#include "Renderer.h"

class SceneManager {

public:

	void Display(const GameManager& _Manager) const;

private:

	void RenderTiles(const GameManager& _Manager) const;
	void RenderLastMove(const GameManager& _Manager) const;
	void RenderCheck(const GameManager& _Manager) const;
	void RenderSelection(const GameManager& _Manager) const;
	void RenderPieces(const GameManager& _Manager) const;
	void RenderMoveMarkers(const GameManager& _Manager) const;
	void RenderCoordinates() const;
	void RenderCapturedPieces(const GameManager& _Manager) const;
	void RenderTextOverlay(const GameManager& _Manager) const;
	void RenderPromotionPanel(const GameManager& _Manager) const;

	void RenderPieceAt(Chess::Piece _Piece, Vec2 _Center, float _Size) const;

	Renderer Draw;
};

#endif
