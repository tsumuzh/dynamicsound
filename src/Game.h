#pragma once
#include <SDL.h>

struct Vector2
{
	float x;
	float y;
};

struct Paddle
{
	Vector2 position;
	int direction;
};

struct Ball
{
	Vector2 position;
	Vector2 velocity;
};

class Game
{
public:
	Game();
	bool Initialize(); // Initialize game
	void RunLoop();	   // Run game loop
	void Shutdown();   // Shutdown game
private:
	void ProcessInput();
	void UpdateGame();
	void GenerateOutput();

	SDL_Window *mWindow;	 // Wwndow
	SDL_Renderer *mRenderer; // 2D renderer
	bool mIsRunning;
	Uint32 mTicksCount;
};