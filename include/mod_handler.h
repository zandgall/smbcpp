#ifndef SMBCPP
#pragma once
#include "../src/zandgall/base/glhelper.h"
#include "../src/zandgall/base/handler.h"
#include "../src/zandgall/base/Sound.h"
#include "../src/zandgall/application/App.h"
//#include "objects.h" // Included by App.h
#else
class App;
//struct GLFWwindow;
//struct ALCcontext;
//struct ALCdevice;
#include "../src/zandgall/base/glhelper.h"
#endif /* SMBCPP */

struct ModHandler {
	App* app = nullptr;
	double* delta = nullptr, * next_delta = nullptr;
	GLFWwindow* glfw_window = nullptr;
	int *window_width = nullptr, *window_height = nullptr;
	ALCcontext* sound_context = nullptr;
	ALCdevice* sound_device = nullptr;
	double *pmouseX = nullptr, *pmouseY = nullptr, *pmouseScroll = nullptr, *mouseX = nullptr, *mouseY = nullptr, *mouseScroll = nullptr;
	bool* pmouseLeft = nullptr, * pmouseRight = nullptr, * pmouseMiddle = nullptr, *mouseLeft = nullptr, *mouseRight = nullptr, *mouseMiddle = nullptr;
	bool* keys = nullptr, * pKeys = nullptr;
};
/*
* _mod_tick is called before ticking every object
* _mod_tick_post is called after ticking every object
* _mod_render is called before every object is rendered
* _mod_render_post is called after every object is rendered
* _mod_render_pre is called at the beginning of the render stage (before framebufferst and shaders are fully prepared)
* _mod_render_end is called at the end of the render stage (after everything is ready to be pushed to the screen)
*/
typedef void (*mod_function)(ModHandler&);