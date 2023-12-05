#include "Game.h"
#include "DynamicSound.h"
#include <vector>
#include <Windows.h>
#include <mmsystem.h>
#include <math.h>

const int thickness = 15;
const float paddleH = 100.0f;
const float ballSpeed = 384.0f;
const float paddleSpeed = 384.0f;

const int ballCount = 1;

Paddle paddle1, paddle2;
std::vector<Ball> balls(ballCount);

Game::Game()
	: mWindow(nullptr),
	  mRenderer(nullptr),
	  mTicksCount(0),
	  mIsRunning(true)
{
}

bool Game::Initialize()
{
	int sdlResult = SDL_Init(SDL_INIT_VIDEO); // Initialize SDL
	if (sdlResult != 0)
	{
		SDL_Log("Failed to initialize SDL : %s", SDL_GetError());
		return false;
	}

	//(Window title, top-left x-pos of window, top-left y-pos, width, height, flag)
	/*
	About Flag
	SDL_WINDOW_FULLSCREEN			Full screen.
	SDL_WINDOW_FULLSCREEN_DESKTOP	Full screen in current desktop resolution. Ignore SDL_CreateWindow width and height.
	SDL_WINDOW_OPENGL				Use OpenGL graphics library.
	SDL_WINDOW_RESIZABLE			User can resize window.
	*/
	mWindow = SDL_CreateWindow("Pong", 100, 100, 1024, 768, 0);
	if (!mWindow)
	{
		SDL_Log("Failed to create window : %s", SDL_GetError());
		return false;
	}

	mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC); //(Pointer to window, Assign graphic driver, flag)

	paddle1.position.x = 10.0f;
	paddle1.position.y = 768.0f / 2.0f;
	paddle2.position.x = 1014.0f;
	paddle2.position.y = 768.0f / 2.0f;

	for (int i = 0; i < ballCount; i++)
	{
		Ball b = balls.at(i);
		b.position.x = static_cast<float>(rand() * 512.0f / RAND_MAX) + 256.0f;
		b.position.y = static_cast<float>(rand() * 512.0f / RAND_MAX) + 128.0f;
		float direction = static_cast<float>(rand() * M_PI / RAND_MAX);
		if (direction > M_PI / 2)
			direction += M_PI / 4;
		else
			direction -= M_PI / 4;
		b.velocity.x = ballSpeed * cosf(direction);
		b.velocity.y = ballSpeed * sinf(direction);
		balls.at(i) = b;
	}

	DS_Initialize();

	return true;
}

void Game::RunLoop()
{
	while (mIsRunning)
	{
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
}

void Game::ProcessInput()
{
	SDL_Event event;
	// Loop if events exist in queue
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			mIsRunning = false;
			break;
		}
	}

	const Uint8 *state = SDL_GetKeyboardState(NULL); // Get keyboard state
	if (state[SDL_SCANCODE_ESCAPE])
		mIsRunning = false; // Shutdown when ESC pressed

	paddle1.direction = 0;
	if (state[SDL_SCANCODE_W])
		paddle1.direction = -1;
	if (state[SDL_SCANCODE_S])
		paddle1.direction = 1;
	paddle2.direction = 0;
	if (state[SDL_SCANCODE_I])
		paddle2.direction = -1;
	if (state[SDL_SCANCODE_K])
		paddle2.direction = 1;
}

void Game::UpdateGame()
{
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16))
		; // 16 is frame time(ms) in 60 fps

	float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;

	if (deltaTime > 0.05f)
		deltaTime = 0.05f;

	mTicksCount = SDL_GetTicks();

	// Update paddle position
	// paddle1
	if (paddle1.direction != 0)
		paddle1.position.y += paddle1.direction * paddleSpeed * deltaTime;
	if (paddle1.position.y < (paddleH / 2.0f + thickness))
		paddle1.position.y = paddleH / 2.0f + thickness;
	else if (paddle1.position.y > (768.0f - paddleH / 2.0f - thickness))
		paddle1.position.y = 768.0f - paddleH / 2.0f - thickness;
	// paddle2
	if (paddle2.direction != 0)
		paddle2.position.y += paddle2.direction *paddleSpeed * deltaTime;
	if (paddle2.position.y < (paddleH / 2.0f + thickness))
		paddle2.position.y = paddleH / 2.0f + thickness;
	else if (paddle2.position.y > (768.0f - paddleH / 2.0f - thickness))
		paddle2.position.y = 768.0f - paddleH / 2.0f - thickness;

	// Update ball position
	for (int i = 0; i < ballCount; i++)
	{
		Ball b = balls.at(i);
		b.position.x += b.velocity.x * deltaTime;
		b.position.y += b.velocity.y * deltaTime;
		if (b.position.y >= 768.0f - thickness && b.velocity.y > 0.0f)
		{
			DS_SineADSR(600, 1, 0, 0, 0.1, 0, 0);
			b.velocity.y *= -1;
		}
		if (b.position.y <= thickness && b.velocity.y < 0.0f)
		{
			DS_SineADSR(600, 1, 0, 0, 0.1, 0, 0);
			b.velocity.y *= -1;
		}
		// Left
		if (b.position.x <= paddle1.position.x + thickness &&
			b.position.y <= paddle1.position.y + paddleH / 2.0f &&
			b.position.y >= paddle1.position.y - paddleH / 2.0f &&
			b.velocity.x < 0.0f)
		{
			b.velocity.x *= -1;
			DS_TriangleADSR(1200, 1, 0, 0, 0.1, 0, 0);
		}
		// Right
		if (b.position.x >= paddle2.position.x - thickness &&
			b.position.y <= paddle2.position.y + paddleH / 2.0f &&
			b.position.y >= paddle2.position.y - paddleH / 2.0f &&
			b.velocity.x > 0.0f)
		{
			b.velocity.x *= -1;
			DS_SawADSR(1500, 1, 0, 0, 0.1, 0, 0);
		}
		if (b.position.x < 0.0f || b.position.x > 1024.0f)
		{
			b.position.x = 512.0f;
			b.position.y = 384.0f;
			float direction = static_cast<float>(rand() * M_PI / RAND_MAX);
			if (direction > M_PI / 2)
				direction += M_PI / 4;
			else
				direction -= M_PI / 4;

			b.velocity.x = ballSpeed * cosf(direction);
			b.velocity.y = ballSpeed * sinf(direction);

			DS_RectADSR(160, 1.0, 0.5, 0.1, 0.2, 0.1, 0.1);
		}
		balls.at(i) = b;
	}

	if (ballCount == 1)
	{
		Ball b = balls.at(0);
		float x = abs(b.position.x - 512.0f) / 512.0f;
		DS_Sine(x, x * x * 1000);
	}
}

void Game::GenerateOutput()
{
	SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255); //(Renderer, R, G, B, A)
	SDL_RenderClear(mRenderer);						 // Clear back buffer

	SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);
	SDL_Rect wall{0, 0, 1024, thickness};
	SDL_RenderFillRect(mRenderer, &wall);
	wall.y = 768 - thickness;
	SDL_RenderFillRect(mRenderer, &wall);

	for (Ball b : balls)
	{
		SDL_Rect ball{
			static_cast<int>(b.position.x - thickness / 2),
			static_cast<int>(b.position.y - thickness / 2),
			thickness,
			thickness};
		SDL_RenderFillRect(mRenderer, &ball);
	}
	SDL_Rect paddle{
		static_cast<int>(paddle1.position.x - thickness / 2),
		static_cast<int>(paddle1.position.y - paddleH / 2),
		thickness,
		static_cast<int>(paddleH)};
	SDL_RenderFillRect(mRenderer, &paddle);
	paddle.x = static_cast<int>(paddle2.position.x - thickness / 2);
	paddle.y = static_cast<int>(paddle2.position.y - paddleH / 2);
	SDL_RenderFillRect(mRenderer, &paddle);

	SDL_RenderPresent(mRenderer); // Switch front buffer and back buffer

	// DS_Queue();
	DS_Play();
}

void Game::Shutdown()
{
	DS_Close();
	SDL_DestroyRenderer(mRenderer);
	SDL_DestroyWindow(mWindow);
	SDL_Quit();
}