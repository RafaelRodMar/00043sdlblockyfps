#include<SDL.h>
#include<time.h>
#include "AssetsManager.h"
#include "InputHandler.h"
#include "game.h"

//the game class
Game* Game::s_pInstance = 0;

Game::Game() {
	m_pRenderer = nullptr;
	m_pWindow = nullptr;
}

Game::~Game() {}

SDL_Window* g_pWindow = 0;
SDL_Renderer* g_pRenderer = 0;

bool Game::init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen) {
	m_gameWidth = width;
	m_gameHeight = height;

	// initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		int flags = SDL_WINDOW_SHOWN;
		if (fullscreen) flags |= SDL_WINDOW_FULLSCREEN;

		// if succeeded create our window
		std::cout << "SDL init success\n";
		m_pWindow = SDL_CreateWindow(title,
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			width, height, flags);
		// if the window creation succeeded create our renderer
		if (m_pWindow != 0)
		{
			std::cout << "window creation success\n";
			m_pRenderer = SDL_CreateRenderer(m_pWindow, -1, 0);
			if (m_pRenderer != 0)
			{
				std::cout << "renderer creation success\n";
				SDL_SetRenderDrawColor(m_pRenderer, 255, 255, 255, 255);
			}
			else
			{
				std::cout << "renderer init fail\n";
				return false;
			}
		}
		else
		{
			std::cout << "window init fail\n";
			return false;
		}
	}
	else
	{
		std::cout << "SDL init fail\n";
		return false; // sdl could not initialize
	}
	std::cout << "init success\n";
	m_bRunning = true;

	//pass the renderer to the assets manager
	AssetsManager::Instance()->renderer = m_pRenderer;

	g_pRenderer = m_pRenderer;
	g_pWindow = m_pWindow;
	return true;
}

void Game::handleEvents() {
	InputHandler::Instance()->update();
}

void Game::update() {
}

void Game::render() {
	// set to black // This function expects Red, Green, Blue and
	// Alpha as color values
	SDL_SetRenderDrawColor(g_pRenderer, 0, 0, 0, 255);
	// clear the window to black
	SDL_RenderClear(g_pRenderer);

	// show the window
	SDL_RenderPresent(g_pRenderer);
}

void Game::quit() {
	m_bRunning = false;
}

int main(int argc, char* args[])
{
	srand(time(nullptr));

	if (Game::Instance()->init("Blockyfps", 100, 100, Game::Instance()->blockWidth * Game::Instance()->nScreenWidth,
		Game::Instance()->blockHeight * Game::Instance()->nScreenHeight, false))
	{
		while (Game::Instance()->running()) {
			Game::Instance()->handleEvents();
			Game::Instance()->update();
			Game::Instance()->render();
		}
	}
	else
	{
		std::cout << "game init failure - " << SDL_GetError() << "\n";
		return -1;
	}

	std::cout << "game closing...\n";

	// clean up SDL
	SDL_DestroyWindow(g_pWindow);
	SDL_DestroyRenderer(g_pRenderer);
	AssetsManager::Instance()->clearAllTextures();
	InputHandler::Instance()->clean();
	SDL_Quit();
	return 0;
}