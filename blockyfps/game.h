#pragma once

#include <vector>
#include <SDL.h>
#include "AssetsManager.h"
#include "InputHandler.h"

class Game {
public:
	static Game* Instance()
	{
		if (s_pInstance == 0)
		{
			s_pInstance = new Game();
			return s_pInstance;
		}
		return s_pInstance;
	}

	SDL_Renderer* getRenderer() const { return m_pRenderer; }

	~Game();

	bool init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen);
	void render();
	void update();
	void handleEvents();
	//void clean();
	void quit();

	bool running() { return m_bRunning; }

	int getGameWidth() const { return m_gameWidth; }
	int getGameHeight() const { return m_gameHeight; }

	//world variables
	int nScreenWidth = 120; //columns blocks
	int nScreenHeight = 40; //rows blocks
	int blockWidth = 5;
	int blockHeight = 7;
	int nMapWidth = 16;
	int nMapHeight = 16;

	float fPlayerX = 14.7f; //player start position
	float fPlayerY = 5.09f;
	float fPlayerA = 0.0f;  //player start rotation
	float fFOV = 3.14159f / 4.0f; //field of view
	float fDepth = 16.0f; //maximum rendering distance
	float fSpeed = 5.0f;  //walking speed

	//world map, # = wall, . = space
	std::string map;
	std::string screen;

private:
	Game();
	static Game* s_pInstance;
	SDL_Window* m_pWindow;
	SDL_Renderer* m_pRenderer;

	bool m_bRunning;
	int m_gameWidth;
	int m_gameHeight;
};
