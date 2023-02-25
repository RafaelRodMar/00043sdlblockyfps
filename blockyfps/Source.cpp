#include<SDL.h>
#include<time.h>
#include<chrono>
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

	//fill map
	map += "#########.......";
	map += "#...............";
	map += "#.......########";
	map += "#..............#";
	map += "#......##......#";
	map += "#......##......#";
	map += "#..............#";
	map += "###............#";
	map += "##.............#";
	map += "#......####..###";
	map += "#......#.......#";
	map += "#......#.......#";
	map += "#..............#";
	map += "#......#########";
	map += "#..............#";
	map += "################";

	//fill screen
	screen = std::string(nScreenWidth*nScreenHeight, ' ');

	g_pRenderer = m_pRenderer;
	g_pWindow = m_pWindow;
	return true;
}

void Game::handleEvents() {
	InputHandler::Instance()->update();

	//counterclockwise rotation
	if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_A)) fPlayerA -= (fSpeed * 0.75f);
	
	//clockwise rotation
	if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_D)) fPlayerA += (fSpeed * 0.75f);

	// Handle Forwards movement & collision
	if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_W))
	{
		fPlayerX += sinf(fPlayerA) * fSpeed;
		fPlayerY += cosf(fPlayerA) * fSpeed;
		if (map[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
		{
			fPlayerX -= sinf(fPlayerA) * fSpeed;
			fPlayerY -= cosf(fPlayerA) * fSpeed;
		}
	}

	// Handle backwards movement & collision
	if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_S))
	{
		fPlayerX -= sinf(fPlayerA) * fSpeed;
		fPlayerY -= cosf(fPlayerA) * fSpeed;
		if (map[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
		{
			fPlayerX += sinf(fPlayerA) * fSpeed;
			fPlayerY += cosf(fPlayerA) * fSpeed;
		}
	}
}

void Game::update() {
	for (int x = 0; x < nScreenWidth; x++)
	{
		// For each column, calculate the projected ray angle into world space
		float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

		// Find distance to wall
		float fStepSize = 0.1f;		  // Increment size for ray casting, decrease to increase										
		float fDistanceToWall = 0.0f; //                                      resolution

		bool bHitWall = false;		// Set when ray hits wall block
		bool bBoundary = false;		// Set when ray hits boundary between two wall blocks

		float fEyeX = sinf(fRayAngle); // Unit vector for ray in player space
		float fEyeY = cosf(fRayAngle);

		// Incrementally cast ray from player, along ray angle, testing for 
		// intersection with a block
		while (!bHitWall && fDistanceToWall < fDepth)
		{
			fDistanceToWall += fStepSize;
			int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
			int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

			// Test if ray is out of bounds
			if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
			{
				bHitWall = true;			// Just set distance to maximum depth
				fDistanceToWall = fDepth;
			}
			else
			{
				// Ray is inbounds so test to see if the ray cell is a wall block
				if (map[nTestX * nMapWidth + nTestY] == '#')
				{
					// Ray has hit wall
					bHitWall = true;

					// To highlight tile boundaries, cast a ray from each corner
					// of the tile, to the player. The more coincident this ray
					// is to the rendering ray, the closer we are to a tile 
					// boundary, which we'll shade to add detail to the walls
					vector<pair<float, float>> p;

					// Test each corner of hit tile, storing the distance from
					// the player, and the calculated dot product of the two rays
					for (int tx = 0; tx < 2; tx++)
						for (int ty = 0; ty < 2; ty++)
						{
							// Angle of corner to eye
							float vy = (float)nTestY + ty - fPlayerY;
							float vx = (float)nTestX + tx - fPlayerX;
							float d = sqrt(vx*vx + vy * vy);
							float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
							p.push_back(make_pair(d, dot));
						}

					// Sort Pairs from closest to farthest
					sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float, float> &right) {return left.first < right.first; });

					// First two/three are closest (we will never see all four)
					float fBound = 0.01;
					if (acos(p.at(0).second) < fBound) bBoundary = true;
					if (acos(p.at(1).second) < fBound) bBoundary = true;
					if (acos(p.at(2).second) < fBound) bBoundary = true;
				}
			}
		}

		// Calculate distance to ceiling and floor
		int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
		int nFloor = nScreenHeight - nCeiling;

		// Shader walls based on distance
		short nShade = ' ';
		if (fDistanceToWall <= fDepth / 4.0f)			nShade = 0x2588;	// Very close	
		else if (fDistanceToWall < fDepth / 3.0f)		nShade = 0x2593;
		else if (fDistanceToWall < fDepth / 2.0f)		nShade = 0x2592;
		else if (fDistanceToWall < fDepth)				nShade = 0x2591;
		else											nShade = ' ';		// Too far away

		if (bBoundary)		nShade = ' '; // Black it out

		for (int y = 0; y < nScreenHeight; y++)
		{
			// Each Row
			if (y <= nCeiling)
				screen[y*nScreenWidth + x] = ' ';
			else if (y > nCeiling && y <= nFloor)
				screen[y*nScreenWidth + x] = nShade;
			else // Floor
			{
				// Shade floor based on distance
				float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
				if (b < 0.25)		nShade = '#';
				else if (b < 0.5)	nShade = 'x';
				else if (b < 0.75)	nShade = '.';
				else if (b < 0.9)	nShade = '-';
				else				nShade = ' ';
				screen[y*nScreenWidth + x] = nShade;
			}
		}
	}
}

void Game::render() {
	// set to black // This function expects Red, Green, Blue and
	// Alpha as color values
	SDL_SetRenderDrawColor(g_pRenderer, 0, 0, 0, 255);
	// clear the window to black
	SDL_RenderClear(g_pRenderer);

	// Display Map
	for(int ny = 0; ny < nScreenHeight; ny++)
		for (int nx = 0; nx < nScreenWidth; nx++)
		{
			Uint8 cr = 255;
			if (screen[ny*nScreenWidth + nx] == '#') cr = 255;
			if (screen[ny*nScreenWidth + nx] == 'x') cr = 204;
			if (screen[ny*nScreenWidth + nx] == '.') cr = 102;
			if (screen[ny*nScreenWidth + nx] == '-') cr = 51;
			if (screen[ny*nScreenWidth + nx] == ' ') cr = 0;

			SDL_SetRenderDrawColor(g_pRenderer, cr, 0, 0, 255);
			SDL_Rect rect;
			rect.x = nx * blockWidth;
			rect.y = ny * blockHeight;
			rect.h = blockHeight;
			rect.w = blockWidth;
			SDL_RenderFillRect(g_pRenderer, &rect);
		}
	screen[((int)fPlayerX + 1) * nScreenWidth + (int)fPlayerY] = 'P';


	// show the window
	SDL_RenderPresent(g_pRenderer);
}

void Game::quit() {
	m_bRunning = false;
}

const int FPS = 60;
const int DELAY_TIME = 1000.0f / FPS;

int main(int argc, char* args[])
{
	srand(time(nullptr));

	Uint32 frameStart, frameTime;

	if (Game::Instance()->init("Blockyfps", 100, 100, Game::Instance()->blockWidth * Game::Instance()->nScreenWidth,
		Game::Instance()->blockHeight * Game::Instance()->nScreenHeight, false))
	{
		while (Game::Instance()->running()) {
			frameStart = SDL_GetTicks(); //initial time

			Game::Instance()->handleEvents();
			Game::Instance()->update();
			Game::Instance()->render();

			frameTime = SDL_GetTicks() - frameStart; //final time - initial time

			if (frameTime < DELAY_TIME)
			{
				SDL_Delay((int)(DELAY_TIME - frameTime)); //wait until complete the 60 fps
			}
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