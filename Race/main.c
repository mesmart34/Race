#include "SDL/SDL.h"
#include "SDL/SDL_main.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#define TITLE "Pseudo 3D"
#define WIDTH 1024
#define HEIGHT 768
#define CAMERA_HEIGHT 50.0
#define HORIZON (int)(HEIGHT * 0.5f)
#define SOLID_LINES 3
#define ROAD_WIDTH SOLID_LINES * 60
#define BORDER_WIDTH 6
#define SOLID_WIDTH 1.2
static SDL_Window* window;
static SDL_Renderer* renderer;

float speed = 1.25f;
float position = 0.0f;
float z_map[HEIGHT];
float h_map[HEIGHT];
float segment_length = 0.035f;

int track_length = 4;
typedef struct
{
	float x;
	float y;
} track_data;
track_data track[] = { {-0.2, 50}, {0.1, 30}, {0.3, 20}, {0, 50} };
int tracker = 0;

float current_track_length = 0.0f;
float current_curve = 0.0f;

typedef struct 
{
	float x;
	float dx;
	float position;
	float dy;
	float y;
} segment_t;

segment_t segments[HEIGHT];
segment_t segment;

track_data GetNextDX()
{
	track_data value = track[tracker];
	value.x /= 5;
	tracker++;
	if (tracker >= track_length)
		tracker = 0;
	return value;
}

void DrawRoadLine(SDL_Renderer* renderer, float curve, float y, float z, bool light)
{
	if(light)
		SDL_SetRenderDrawColor(renderer, 231, 231, 231, 255);
	else
		SDL_SetRenderDrawColor(renderer, 219, 199, 180, 255);
	SDL_RenderDrawLineF(renderer, 0, y, WIDTH, y);
	//ROAD
	if (light)
		SDL_SetRenderDrawColor(renderer, 126, 126, 126, 255);
	else
		SDL_SetRenderDrawColor(renderer, 148, 148, 148, 255);
	float left = ((-ROAD_WIDTH / 2) / z) + curve + WIDTH / 2;
	float right = ((ROAD_WIDTH / 2) / z) + curve + WIDTH / 2;
	SDL_RenderDrawLineF(renderer, left, y, right, y);
	
	SDL_SetRenderDrawColor(renderer, 231, 231, 231, 255);
	for (int i = 0; i < SOLID_LINES; i++)
	{
		float solid_x_left =  ((i + 1) * (ROAD_WIDTH / SOLID_LINES) - SOLID_WIDTH - ROAD_WIDTH / 2) / z + curve + WIDTH / 2;
		float solid_x_right = ((i + 1) * (ROAD_WIDTH / SOLID_LINES) + SOLID_WIDTH - ROAD_WIDTH / 2) / z + curve + WIDTH / 2;
		if (light)
			SDL_RenderDrawLineF(renderer, solid_x_left, y, solid_x_right, y);
		else
			SDL_SetRenderDrawColor(renderer, 148, 148, 148, 255);
	}
	float left_border_x_left = (-ROAD_WIDTH / 2) / z + WIDTH / 2 + curve;
	float left_border_x_right = (-ROAD_WIDTH / 2 + BORDER_WIDTH) / z + WIDTH / 2 + curve;
	SDL_SetRenderDrawColor(renderer, 231, 231, 231, 255);
	if(!light)
		SDL_RenderDrawLineF(renderer, left_border_x_left, y, left_border_x_right, y);

	float right_border_x_left = (ROAD_WIDTH / 2) / z + WIDTH / 2 + curve;
	float right_border_x_right = (ROAD_WIDTH / 2 - BORDER_WIDTH) / z + WIDTH / 2 + curve;
	SDL_SetRenderDrawColor(renderer, 231, 231, 231, 255);
	if (!light)
		SDL_RenderDrawLineF(renderer, right_border_x_left, y, right_border_x_right, y);
}

void Init()
{
	for (int index = 0; index < HEIGHT; index++)
	{
		z_map[index] = -(1.0f + CAMERA_HEIGHT) / (index - ((HEIGHT) * 0.51));
	}
	track_data data = GetNextDX();
	current_track_length = data.y;
	segment.dx = data.x;
	for (int index =  0; index < 100; index++)
	{
		segments[index].y = segments[index - 1].y + 0.05;
		printf("%f\n", segments[index].y);
	}
	for (int index = 100; index < HEIGHT - 100; index++)
	{
		segments[index].y = segments[79].y;
		printf("%f\n", segments[index].y);
	}
}

void Start()
{
	Init();
}

void Update()
{
	
	position += speed;
}

void Render(SDL_Renderer* renderer)
{
	segments[0].dx = 0.01;
	segments[0].dy = 0.01;
	segment_t base = segments[0];
	int z_index = 0;
	float last_h = segments[0].y;
	float x = 0;
	float dx = base.dx;
	float y = 0;
	float dy = base.dy;
	for (int index = 0; index < HORIZON; index++)
	{
		dy += base.dy;
		y += dy;
		segments[index].y = sinf(position * 0.05 + index) * 0.5;
		segments[index + 1].dy = sinf(position * 0.05 + (index + 1)) * 0.5;
		z_index++;
		if (segments[index + 1].y > segments[index].y) // UP HILLS
		{
			if (segments[index + 1].y - last_h > 1.0f)
			{
				z_index--;
				last_h = segments[index].y;
			}
		}
		else if(segments[index + 1].y < segments[index].y) //DOWN HILLS
		{
			if (last_h - segments[index + 1].y > 1.0f)
			{
				z_index++;
				last_h = segments[index].y;
			}
		}
		float y = HEIGHT - index;
		float z = z_map[z_index];
		x += dx * z;
		dx += base.dx;
		float curve = x;
		bool light = (int)(roundf((z - position) / segment_length)) % 2 == 0;
		DrawRoadLine(renderer, curve, y, z, light);
	}
}

int SDL_main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	bool running = true;
	SDL_Event event;
	Start();
	while (running == 1)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
				running = 0;
		}
		Update();
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);
		Render(renderer);
		SDL_RenderPresent(renderer);
		SDL_Delay(1000 / 60);
	}
	return 0;
}