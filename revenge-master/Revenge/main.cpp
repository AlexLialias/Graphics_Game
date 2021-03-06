#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_mixer.h"
#include <iostream>
#include <chrono>
#include "GLEW\glew.h"
#include "Renderer.h"
#include <thread>         // std::this_thread::sleep_for
#include "Tools.h"
#define STBDS_NO_SHORT_NAMES
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "text_rendering.h"
#include "particle_system.h"
using namespace std;

//Screen attributes
SDL_Window * window;
Mix_Music *music = NULL;
int music_volume = 5;
//OpenGL context 
SDL_GLContext gContext;
const int SCREEN_WIDTH = 1380;	//800;	//640;
const int SCREEN_HEIGHT = 1024;	//600;	//480;
int money = 1000;
//int speed = 1;

//Event handler
SDL_Event event;

Renderer * renderer = nullptr;

void ToggleFullscreen(SDL_Window* Window) {
    Uint32 FullscreenFlag = SDL_WINDOW_FULLSCREEN;
    bool IsFullscreen = SDL_GetWindowFlags(Window) & FullscreenFlag;
    SDL_SetWindowFullscreen(Window, IsFullscreen ? 0 : FullscreenFlag);
    SDL_ShowCursor(IsFullscreen);
}


void func()
{
	system("pause");
}

// initialize SDL and OpenGL
bool init()
{
	if (0)
	{
		SDL_version compiled;
		SDL_version linked;

		SDL_VERSION(&compiled);
		SDL_GetVersion(&linked);
		printf("We compiled against SDL version %d.%d.%d ...\n",
			compiled.major, compiled.minor, compiled.patch);
		printf("But we are linking against SDL version %d.%d.%d.\n",
			linked.major, linked.minor, linked.patch);
	}
	//Initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		return false;
	}


	int audio_rate = 22050; Uint16 audio_format = AUDIO_S16SYS; int audio_channels = 2; int audio_buffers = 4096; 
if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0) { fprintf(stderr, "Unable to initialize audio: %s\n", Mix_GetError()); exit(1); } 


	// use Double Buffering
	if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) < 0)
		cout << "Error: No double buffering" << endl;

	// set OpenGL Version (3.3)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	// Create Window
	window = SDL_CreateWindow("Revenge of the Skeleton-Pirates of K'hurr", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (window == NULL)
	{
		printf("Could not create window: %s\n", SDL_GetError());
		return false;
	}

	//Create OpenGL context
	gContext = SDL_GL_CreateContext(window);
	if (gContext == NULL)
	{
		printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	// Disable Vsync
	if (SDL_GL_SetSwapInterval(0) == -1)
		printf("Warning: Unable to disable VSync! SDL Error: %s\n", SDL_GetError());

	// Initialize GLEW
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		printf("Error loading GLEW\n");
		return false;
	}
	// some versions of glew may cause an opengl error in initialization
	glGetError();

	renderer = new Renderer();
	bool engine_initialized = renderer->Init(SCREEN_WIDTH, SCREEN_HEIGHT);

	music = Mix_LoadMUS( "../Data/Sounds/pvz.mp3");
	if (music == nullptr)return 666;
	//atexit(func);

	return engine_initialized;
}


void clean_up()
{
	delete renderer;

	SDL_GL_DeleteContext(gContext);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

int main(int argc, char *argv[])
{
	bool fullscreen = false;
	bool antialiasing = false;
	bool show_options = false;
	//Initialize
	if (init() == false)
	{
		system("pause");
		return EXIT_FAILURE;
	}
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForOpenGL(window, gContext);
    ImGui_ImplOpenGL3_Init();
	//Quit flag
	bool quit = false;
	bool mouse_button_pressed = false;
	glm::vec2 prev_mouse_position(0);

	auto simulation_start = chrono::steady_clock::now();
	Mix_PlayMusic(music,-1);
	Mix_Chunk* meteor = Mix_LoadWAV("../Data/Sounds/meteor.mp3");
	//printf("Mix_PlayMusic: %s\n", Mix_GetError());
	// Wait for user exit
	while (quit == false)
	{
		Mix_VolumeMusic(music_volume);
		// While there are events to handle
		while (SDL_PollEvent(&event))
		{

			if (fullscreen)ToggleFullscreen(window);
			if (event.type == SDL_QUIT)
			{
				quit = true;
			}
			else if (event.type == SDL_KEYDOWN)
			{
				// Key down events
				if (event.key.keysym.sym == SDLK_ESCAPE) quit = true;
				else if (event.key.keysym.sym == SDLK_r) renderer->ReloadShaders();
				else if (event.key.keysym.sym == SDLK_t) renderer->SetRenderingMode(Renderer::RENDERING_MODE::TRIANGLES);
				else if (event.key.keysym.sym == SDLK_l) renderer->SetRenderingMode(Renderer::RENDERING_MODE::LINES);
				else if (event.key.keysym.sym == SDLK_p) renderer->SetRenderingMode(Renderer::RENDERING_MODE::POINTS);
				else if (event.key.keysym.sym == SDLK_w )
				{
					renderer->CameraMoveForward(true);
				}
				else if (event.key.keysym.sym == SDLK_s)
				{
					renderer->CameraMoveBackWard(true);
				}	
				else if (event.key.keysym.sym == SDLK_a)
				{
					renderer->CameraMoveLeft(true);
				}
				else if (event.key.keysym.sym == SDLK_d)
				{
					renderer->CameraMoveRight(true);
				}
			
			}
			else if (event.type == SDL_KEYUP)
			{
				if (event.key.keysym.sym == SDLK_w)
				{
					renderer->CameraMoveForward(false);
				}
				else if (event.key.keysym.sym == SDLK_s)
				{
					renderer->CameraMoveBackWard(false);
				}
				else if (event.key.keysym.sym == SDLK_a)
				{
					renderer->CameraMoveLeft(false);
				}
				else if (event.key.keysym.sym == SDLK_d)
				{
					renderer->CameraMoveRight(false);
				}
				else if(event.key.keysym.sym == SDLK_TAB) {
					show_options = !show_options;
				}
				else if (event.key.keysym.sym == SDLK_DOWN) {
					renderer->move_green_plane(glm::vec3(0,0,-2));
				}
				else if (event.key.keysym.sym == SDLK_LEFT) {
					renderer->move_green_plane(glm::vec3(2,0,0));
				}
				else if (event.key.keysym.sym == SDLK_UP) {
					renderer->move_green_plane(glm::vec3(0,0,2));
				}
				else if (event.key.keysym.sym == SDLK_RIGHT) {
					renderer->move_green_plane(glm::vec3(-2,0,0));
				}
				else if (event.key.keysym.sym == SDLK_b)
				{
					renderer->BuildTower();
				}
				else if (event.key.keysym.sym == SDLK_v)
				{
					renderer->RemoveTower();
				}
				else if (event.key.keysym.sym == SDLK_c) {
					renderer->SpawnPirate();
				}else if (event.key.keysym.sym == SDLK_n) {
					if (renderer->SpawnMeteor()) {
						Mix_PlayChannel(1, meteor, 0);
					}
				}


			}
			else if (event.type == SDL_MOUSEMOTION&& show_options == false)
			{
				int x = event.motion.x;
				int y = event.motion.y;
				if (mouse_button_pressed)
				{
					renderer->CameraLook(glm::vec2(x, y) - prev_mouse_position);
					prev_mouse_position = glm::vec2(x, y);
				}
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
			{
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					int x = event.button.x;
					int y = event.button.y;
					mouse_button_pressed = (event.type == SDL_MOUSEBUTTONDOWN);
					prev_mouse_position = glm::vec2(x, y);
				}
			}
			else if (event.type == SDL_MOUSEWHEEL)
			{
				int x = event.wheel.x;
				int y = event.wheel.y;
			}
			else if (event.type == SDL_WINDOWEVENT)
			{
				if (event.window.event == SDL_WINDOWEVENT_RESIZED)
				{
					renderer->ResizeBuffers(event.window.data1, event.window.data2);
				}
			}
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplSDL2_NewFrame(window);
			ImGui::NewFrame();
			if(show_options)
			{
				ImGui::Begin("options",0, ImGuiWindowFlags_AlwaysAutoResize);
				ImGui::Checkbox("anti-aliasing", &antialiasing);
				ImGui::Checkbox("fullscreen", &fullscreen);
				ImGui::SliderInt("volume", &music_volume, 0.0f, MIX_MAX_VOLUME);
				ImGui::SliderInt("speed", &renderer->speed, -32.0f, 32.0f);
				ImGui::End();
			}
			ImGui::Render();
		}
		// Compute the ellapsed time
		auto simulation_end = chrono::steady_clock::now();
		float dt = chrono::duration <float>(simulation_end - simulation_start).count(); // in seconds
		simulation_start = chrono::steady_clock::now();

		// Update
		renderer->Update(dt);

		// Draw
		renderer->Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());	
		//Update screen (swap buffer for double buffering)
		SDL_GL_SwapWindow(window);
	}

	//Clean up
	clean_up();

	return 0;
}
