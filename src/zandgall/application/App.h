#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H
#include "objects.h"
#include <vector>
#include <windows.h>

#define GAME_SCALE 4

#define NES_WIDTH 256
#define NES_HEIGHT 224

class App {
public:
	static App* instance;
	Mario player;
	std::vector<Object*> tiles;
	std::map<std::string, Sprite*> sprites;
	bool paused = false; //TODO enact "paused" condition, find out what stops when game is paused.
	glm::vec2 worldOffset = glm::vec2(0);
	unsigned int getPalette(unsigned int index);
	void loadWorld(int major, int minor);
	Sprite* loadSprite(const std::string& path, bool save = true);
	void loadObject(const std::string& path);
	void loadMod(const std::string& path);
	template<typename T>
	T getModProcess(const std::string& mod, const std::string& process_name);
	Object* createObjectFromDefinition(std::string name, float x = 0, float y = 0);
	void loadMarioStageDefinition(const std::string& path);
	const Mario::Stage* getMarioStageDefinition(std::string name);
	Sprite* getMarioStageSprite(Mario* mario, double frame_index);
	void drawSprite(Sprite* sprite, int x, int y, bool flipped = false, int cutoff_w = -1, int cutoff_h = -1);
	App();

	~App() {
		
	}

	void tick(double delta, double next_delta);
	void render();
private:
	std::map<std::string, Object*> object_definitions = std::map<std::string, Object*>();
	std::map<std::string, Mario::Stage*> mario_stages = std::map<std::string, Mario::Stage*>();
	std::map<std::string, HMODULE> mods = std::map<std::string, HMODULE>();
	std::vector<unsigned int> palettes = std::vector<unsigned int>();
	Sprite* _loadSprite(nbt::compound&);
	inline void drawObject(Object* obj);
	unsigned int palette;
	unsigned int bgLayer, objLayer;
	unsigned int paletteTextureFile, tilesetTextureFile, tileTestShader,
		bgLayerFB, objLayerFB, tileShader, blitShader;
	void tickObject(Object* obj, double delta, double next_delta);
};
#endif