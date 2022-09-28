#define NBT_INCLUDE
#define NBT_COMPILE
#include <nbt/nbt.hpp>
#define NBT_GZNBT_INCLUDE
#include <loadgz/gznbt.h>
#include "App.h"
#include "../base/glhelper.h"
#include "../base/handler.h"

#include "objects.h"

// Modding
#include <libloaderapi.h>

Sprite title, copyright;
Object titlero, copyrightro;

ModHandler handler;

App* App::instance = nullptr;

void App::loadWorld(int major, int minor) {
	nbt::compound world = nbt::compound();
	std::ifstream in = std::ifstream(std::string("" + major) + "/" + std::string("" + minor) + ".nbt", std::ios::binary);
	in.seekg(0, std::ios::end);
	size_t len = in.tellg();
	in.seekg(0, std::ios::beg);
	std::vector inbytes = std::vector<char>(len);
	in.read(&inbytes[0], len);

	std::vector file = std::vector<char>();
	nbt::inflate(&inbytes[0], len, &file);

	world.load(&file[0], 0);
}

Sprite* App::_loadSprite(nbt::compound& data) {
	Sprite* out = new Sprite();
	if (data.has("version") && data["version"]._string() == "0.1") {
		out->width = data["width"]._int();
		out->height = data["height"]._int();
		out->tiles = new glm::ivec2[out->width * out->height];
		for (int i = 0; i < out->width; i++)
			for (int j = 0; j < out->height; j++)
				out->tiles[i + j * out->width] = glm::vec2(data["tilemap"]._intarray()[(i + j * out->width) * 2], data["tilemap"]._intarray()[(i + j * out->width) * 2 + 1]);
		if (data.has("flipmap")) {
			out->flipmap = new bool[out->width * out->height];
			for (int i = 0; i < out->width; i++)
				for (int j = 0; j < out->height; j++)
					out->flipmap[i + j * out->width] = data["flipmap"]._bytearray()[i + j * out->width];
		}
		if (data["palette"]["type"]._string() == "built-in") {
			out->palette = getPalette(data["palette"]["index"]._int());
		}
		//name = data["name"]._string();
	}
	return out;
}

Sprite* App::loadSprite(const std::string& path, bool save) {
	
	nbt::compound spritedata = nbt::compound();
	std::ifstream in = std::ifstream(path, std::ios::binary);
	in.seekg(0, std::ios::end);
	size_t len = in.tellg();
	in.seekg(0, std::ios::beg);
	std::vector inbytes = std::vector<char>(len);
	in.read(&inbytes[0], len);

	std::vector file = std::vector<char>();
	nbt::inflate(&inbytes[0], len, &file);

	spritedata.load(&file[0], 0);

	Sprite* out = _loadSprite(spritedata);
	std::string name = path;
	if (spritedata.has("version") && spritedata["version"]._string() == "0.1")
		name = spritedata["name"]._string();
	else
		std::cout << "Invalid version! Tag was " << spritedata["version"]->compilation() << std::endl;

	if (save) {
		if (sprites.find(name) != sprites.end())
			delete sprites[name]; // Override any existing pointer with that name
		sprites.insert(std::make_pair(name, out));
	}
	return out;
}

void App::loadObject(const std::string& path) {
	nbt::compound objdata = nbt::compound();
	std::ifstream in = std::ifstream(("res/objects/"+path).c_str(), std::ios::binary);
	in.seekg(0, std::ios::end);
	size_t len = in.tellg();
	in.seekg(0, std::ios::beg);
	std::vector inbytes = std::vector<char>(len);
	in.read(&inbytes[0], len);

	std::vector file = std::vector<char>();
	nbt::inflate(&inbytes[0], len, &file);


	objdata.load(&file[0], 0);
	std::string name;
	Object* def = new Object();
	if (objdata.has("version") && objdata["version"]._string() == "0.1") {
		nbt::list& sprites = objdata["sprites"]._list();
		for (int i = 0; i < sprites.tags.size(); i++) {
			def->sprites.push_back(Object::ObjectSprite());
			def->sprites[i].sprite = this->sprites[sprites[i]["name"]._string()];
			def->sprites[i].offset = glm::ivec2(sprites[i]["x"]._int(), sprites[i]["y"]._int());
			if (sprites[i]._compound().has("width"))
				def->sprites[i].size.x = sprites[i]["width"]._int();
			else def->sprites[i].size.x = def->sprites[i].sprite->width;
			if (sprites[i]._compound().has("height"))
				def->sprites[i].size.y = sprites[i]["height"]._int();
			else def->sprites[i].size.y = def->sprites[i].sprite->height;
		}
		//nbt::compound& physics_attributes = objdata["physics_attributes"]._compound();
		if (objdata.has("physics_attributes")) {
			if (objdata["physics_attributes"].has("isStatic"))
				def->physics_attributes.isStatic = objdata["physics_attributes"]["isStatic"]._byte();
			if (objdata["physics_attributes"].has("isSolid"))
				def->physics_attributes.isSolid = objdata["physics_attributes"]["isSolid"]._byte();
			if (objdata["physics_attributes"].has("gravityAffected"))
				def->physics_attributes.gravityAffected = objdata["physics_attributes"]["gravityAffected"]._byte();
			if (objdata["physics_attributes"].has("hitbox")) {
				def->physics_attributes.hitbox.x = objdata["physics_attributes"]["hitbox"]["x"]._float();
				def->physics_attributes.hitbox.y = objdata["physics_attributes"]["hitbox"]["y"]._float();
				def->physics_attributes.hitbox.z = objdata["physics_attributes"]["hitbox"]["w"]._float();
				def->physics_attributes.hitbox.w = objdata["physics_attributes"]["hitbox"]["h"]._float();
			}
		}
		if (objdata.has("type")) {
			def->type.name = objdata["type"]["name"]._string();
		}
		if (objdata.has("overrides")) {
			if (objdata["overrides"].has("tick"))
				def->overrides.tick = objdata["overrides"]["tick"]._byte();
			if (objdata["overrides"].has("collision"))
				def->overrides.collision = objdata["overrides"]["collision"]._byte();
			if (objdata["overrides"].has("render"))
				def->overrides.render = objdata["overrides"]["render"]._byte();
		}
		def->layer = objdata["layer"]._int();
#define funccheck(name, type) \
		if (objdata.has(#name)) {\
			nbt::list& li = objdata[#name]._list();\
			for (auto i : li.tags) {\
				def->name.push_back(getModProcess<type>(i["dll"]._string(), i["function_name"]._string()));\
			}\
		}

		funccheck(during_tick, tick_function);
		funccheck(post_collision, tick_function);

		funccheck(touches_mario, mario_interaction_function);
		funccheck(touches_object, interaction_function);

		funccheck(touches_floor, interaction_function);
		funccheck(touches_ceil, interaction_function);
		funccheck(touches_wall, interaction_function);
		funccheck(touches_wall_left, interaction_function);
		funccheck(touches_wall_right, interaction_function);

		funccheck( during_render, tick_function);
		
		
		name = objdata["name"]._string();
	}
	object_definitions.insert(std::make_pair(name, def));

}

template<typename T>
T App::getModProcess(const std::string& mod, const std::string& function_name) {
	return (T)GetProcAddress(mods[mod], function_name.c_str());
}

void App::loadMod(const std::string& path) {
	mods.insert(std::make_pair(path, LoadLibraryA(("mods/"+path).c_str())));
}

Object* App::createObjectFromDefinition(std::string name, float x, float y) {
	tiles.push_back(new Object(*object_definitions[name]));
	tiles[tiles.size() - 1]->pos = glm::vec2(x, y);
	return tiles[tiles.size() - 1];
}

void App::loadMarioStageDefinition(const std::string& path) {
	nbt::compound objdata = nbt::compound();
	std::ifstream in = std::ifstream(("res/mario stages/" + path).c_str(), std::ios::binary);
	in.seekg(0, std::ios::end);
	size_t len = in.tellg();
	in.seekg(0, std::ios::beg);
	std::vector inbytes = std::vector<char>(len);
	in.read(&inbytes[0], len);

	std::vector file = std::vector<char>();
	nbt::inflate(&inbytes[0], len, &file);


	objdata.load(&file[0], 0);
	std::string name;
	Mario::Stage* def = new Mario::Stage();
	if (objdata.has("version") && objdata["version"]._string() == "0.1") {
//#define funccheck(name, type) \
//		if (objdata[#name].value) {\
//			nbt::list& li = objdata[#name]._list();\
//			for (auto i : li.tags) {\
//				def->name.push_back(getModProcess<type>(i["dll"]._string(), i["function_name"]._string()));\
//			}\
//		}
		
		def->hitbox.x = objdata["hitbox"]["x"]._float();
		def->hitbox.y = objdata["hitbox"]["y"]._float();
		def->hitbox.z = objdata["hitbox"]["w"]._float();
		def->hitbox.w = objdata["hitbox"]["h"]._float();

		def->palette = objdata["palette"]._int();

		def->name = objdata["name"]._string();
		name = objdata["name"]._string();

		def->custom_data = (objdata["custom_data"]._compound());

		if (getMarioStageDefinition(objdata["on_damage"]._string()))
			def->on_damage = getMarioStageDefinition(objdata["on_damage"]._string());
		else
			def->custom_data.add(new nbt::stringtag(objdata["on_damage"]._string(), "_on_damage_pre_set_"));

		if (getMarioStageDefinition(objdata["on_powerup"]._string()))
			def->on_damage = getMarioStageDefinition(objdata["on_powerup"]._string());
		else
			def->custom_data.add(new nbt::stringtag(objdata["on_powerup"]._string(), "_on_powerup_pre_set_"));


		nbt::list& anm = objdata["animations"]._list();
		for (int i = 0; i < anm.tags.size(); i++) {
			def->animations.push_back(Mario::Animation());
			def->animations[i].animation_length = anm[i]["length"]._double();
			nbt::list& sprites = anm[i]["sprites"]._list();
			for (int j = 0; j < sprites.tags.size(); j++) {
				if (sprites[j]["type"]._string()._Equal("given")) {
					def->animations[i].sprites.push_back(_loadSprite(sprites[j]._compound()));
				}
			}
			//if (anm[i]["custom_timing_func"].value)
			//	getModProcess<mario_function>(anm[i]["custom_timing_func"]["dll"]._string(), anm[i]["custom_timing_func"]["function_name"]._string());
		}
		def->current_animation = getModProcess<animation_index_grabber>(objdata["current_animation"]["dll"]._string(), objdata["current_animation"]["function_name"]._string());
	}
	if (mario_stages.find(name) != mario_stages.end()) {
		if (mario_stages[name] != nullptr)
			delete mario_stages[name];
		mario_stages[name] = def;
	} else
		mario_stages.insert(std::make_pair(name, def));

}

Sprite* App::getMarioStageSprite(Mario* mario, double frame_index) {
	Mario::Animation& anim = mario->stage.animations[mario->stage.current_animation(mario, handler)];
	return anim.sprites[int(frame_index*anim.sprites.size())];
}

const Mario::Stage* App::getMarioStageDefinition(std::string name) {
	return mario_stages[name];
}

void _tickobj(Object* obj, double delta) {
	obj->vel += obj->force * delta;
	obj->pos += obj->vel * delta;
	double &next_delta = *handler.next_delta;
	double factor = delta / next_delta;
	if (obj->physics_attributes.gravityAffected && !obj->physics_attributes.collisions.z)
		obj->vel.y += 30 * factor;

	obj->vel.x = min(max(obj->vel.x, -255 * factor), 255 * factor);
	obj->vel.y = min(max(obj->vel.y, -255 * factor), 255 * factor);
}

void _colobj(Object* obj, double delta, double next_delta, std::vector<Object*>& tiles) {
	obj->physics_attributes.collisions = glm::bvec4(0);
	glm::vec4 hb = (obj->physics_attributes.hitbox + glm::vec4(obj->pos, 0, 0));
	float MARGIN = 0.01f;
	if (AABB(hb, (App::instance->player.physics_attributes.hitbox + glm::vec4(App::instance->player.pos + App::instance->player.vel * delta, 0, 0))))
		for (auto func : obj->touches_mario)
			func(obj, &App::instance->player, handler);

	for (int i = 0; i < tiles.size(); i++) {

		glm::vec4 tilehb = (tiles[i]->physics_attributes.hitbox + glm::vec4(tiles[i]->pos + tiles[i]->vel * delta, 0, 0));
		if (AABB(hb, tilehb))
			for (auto func : obj->touches_object)
				func(obj, tiles[i], handler);
			//obj->touches_object(&obj, &tiles[i], handler);
		if (!tiles[i]->physics_attributes.isSolid || tiles[i] == obj)
			continue;
		if (!obj->physics_attributes.collisions.w && AABB(glm::vec4(hb.x + obj->vel.x * delta, hb.y + MARGIN, hb.z - MARGIN - (hb.x - obj->pos.x + obj->vel.x * delta), hb.w - 2 * MARGIN), tilehb)) {
			obj->physics_attributes.collisions.w = true;
			obj->vel.x = (tilehb.x + tilehb.z + tiles[i]->vel.x * delta - hb.x) / next_delta;
			for (auto func : obj->touches_wall_left)
				func(obj, tiles[i], handler);
			for (auto func : obj->touches_wall)
				func(obj, tiles[i], handler);
		}
		if (!obj->physics_attributes.collisions.y && AABB(glm::vec4(hb.x + hb.z, hb.y + MARGIN, obj->vel.x * delta, hb.z - 2 * MARGIN), tilehb)) {
			obj->physics_attributes.collisions.y = true;
			obj->vel.x = (tilehb.x + tiles[i]->vel.x * delta - hb.x - hb.z) / next_delta;
			for (auto func : obj->touches_wall_right)
				func(obj, tiles[i], handler);
			for (auto func : obj->touches_wall)
				func(obj, tiles[i], handler);
		}
		if (!obj->physics_attributes.collisions.x && AABB(glm::vec4(hb.x + MARGIN, hb.y + obj->vel.y * delta, hb.z - 2 * MARGIN, hb.w - MARGIN - (hb.y - obj->pos.y + obj->vel.y * delta)), tilehb)) {
			obj->physics_attributes.collisions.x = true;
			obj->vel.y = (tilehb.y + tilehb.w + tiles[i]->vel.y * delta - hb.y) / next_delta;
			for (auto func : obj->touches_ceil)
				func(obj, tiles[i], handler);
		}
		if (!obj->physics_attributes.collisions.z && AABB(glm::vec4(hb.x + MARGIN, hb.y + hb.w, hb.z - 2 * MARGIN, obj->vel.y * delta), tilehb)) {
			obj->physics_attributes.collisions.z = true;
			obj->vel.y = (tilehb.y + tiles[i]->vel.y * delta - hb.y - hb.w) / next_delta;
			for (auto func : obj->touches_ceil)
				func(obj, tiles[i], handler);
		}
	}
}

void App::tickObject(Object* obj, double delta, double next_delta) {
	if(!obj->overrides.tick)
		_tickobj(obj, delta);
	for (auto func : obj->during_tick)
		func(obj, handler);
	if (!obj->overrides.collision)
		_colobj(obj, delta, next_delta, tiles);
	for (auto func : obj->post_collision)
		func(obj, handler);
}

inline glm::mat3 uvOfTile(int x, int y, int w = 8, int h = 8) {
	return glm::mat3(w / 256.0, 0, 0, 0, h / 256.0, 0, x / 32.0, y / 32.0, 1);
}

inline void App::drawObject(Object* ro) {
	for (auto pair : ro->sprites) {
		switch (pair.edge_continue_mode) {
		case Object::ObjectSprite::SMB_TRANSPARENT:
			break;
		case Object::ObjectSprite::SMB_REPEAT:
			for(int i = pair.offset.x; i < pair.offset.x + pair.size.x; i+=pair.sprite->width*8)
				for (int j = pair.offset.y; j < pair.offset.y + pair.size.y; j+=pair.sprite->height*8) {
					int t_width = min(pair.sprite->width*8, pair.size.x + pair.offset.x - i);
					int t_height = min(pair.sprite->height*8, pair.size.y + pair.offset.y - j);
					drawSprite(pair.sprite, ro->pos.x+i, ro->pos.y+j, false, t_width, t_height);
				}
			break;
		}
		drawSprite(pair.sprite, ro->pos.x + pair.offset.x, ro->pos.y + pair.offset.y);
	}
}

void App::drawSprite(Sprite* sprite, int x, int y, bool flipped, int cutoff_w, int cutoff_h) {
	for (int i = 0; i < sprite->width; i++)
		for (int j = 0; j < sprite->height; j++) {
			if (sprite->tiles[j * sprite->width + i].x < 0 || sprite->tiles[j * sprite->width + i].y < 0)
				continue;
			glActiveTexture(GL_TEXTURE1);
			if (sprite->palette == 0)
				glBindTexture(GL_TEXTURE_2D, palette);
			else glBindTexture(GL_TEXTURE_2D, sprite->palette);

			int f_width = 8;
			if (cutoff_w != -1)
				f_width = min(8, cutoff_w - 8 * i);
			if (f_width <= 0)
				continue;

			int f_height = 8;
			if (cutoff_h != -1)
				f_height = min(8, cutoff_h - 8 * j);

			if (f_height <= 0)
				continue;

			glActiveTexture(GL_TEXTURE0);
			if(flipped)
				uniMat("transform", rect(8 * (sprite->width - i) + (int)round(x - worldOffset.x), 8 * j + (int)round(y - worldOffset.y), -f_width, f_height));
			else
				uniMat("transform", rect(8 * i + (int)(x - worldOffset.x), 8 * j + (int)(y - worldOffset.y), f_width, f_height));
			auto curTile = sprite->tiles[j * sprite->width + i];
			if(sprite->flipmap && sprite->flipmap[j * sprite->width + i]) // Individual tile flipping
				uniMat("uvtransform", uvOfTile(curTile.x+(1), curTile.y, -f_width, f_height));
			else
				uniMat("uvtransform", uvOfTile(curTile.x, curTile.y, f_width, f_height));
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}
}

inline void setTile(Sprite* sprite, int x, int y, int tilex, int tiley) {
	if (sprite->tiles == nullptr)
		//return;
		sprite->tiles = new glm::ivec2(sprite->width * sprite->height);
	if (x >= sprite->width || y >= sprite->height || x < 0 || y < 0)
		return;
	sprite->tiles[y * sprite->width + x] = glm::vec2(tilex, tiley);
}

App::App() {
	instance = this;
	loadFont("res/fonts/robotomono.ttf", "roboto", 48);
	// Load palette file
	paletteTextureFile = loadTexture("res/palettes/palette.png", GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	tilesetTextureFile = loadTexture("res/tilesets/tileset.png", GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	tileShader = loadShader("res/shaders/tile.shader");
	blitShader = loadShader("res/shaders/blit.shader");
	auto genPalTex = [](unsigned int* texture, unsigned char a, unsigned char b, unsigned char c, unsigned char d) {
		glGenTextures(1, texture);
		glBindTexture(GL_TEXTURE_2D, *texture);
		const unsigned char startingPalette[] = {
			a, 0, 0,
			b, 0, 0,
			c, 0, 0,
			d, 0, 0 };
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 4, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, startingPalette);
		glBindTexture(GL_TEXTURE_2D, 0);
	};
	for (int i = 0; i < 8; i++)
		palettes.push_back(0);
	genPalTex(&palettes[0], 34, 15, 39, 24);
	genPalTex(&palettes[1], 13, 25, 32, 39);
	genPalTex(&palettes[2], 13, 15, 32, 39);
	genPalTex(&palettes[3], 13, 13, 54, 23);
	genPalTex(&palettes[4], 34, 41, 25, 13);
	genPalTex(&palettes[5], 13, 54, 23, 13);
	genPalTex(&palettes[6], 13, 32, 34, 13);
	genPalTex(&palettes[7], 13, 7, 23, 13);
	
	tileTestShader = loadShader("res/shaders/tiletest.shader");

	// Set title
	titlero.physics_attributes.isStatic = false;
	titlero.sprites.push_back(Object::ObjectSprite(loadSprite("res/sprites/title.nbt"), glm::ivec2(0)));
	titlero.pos = glm::vec2(40, 24);

	// Set copyright	
	copyrightro.sprites.push_back(Object::ObjectSprite(loadSprite("res/sprites/copyright.nbt"), glm::ivec2(0)));
	copyrightro.pos = glm::vec2(104, 112);

	// Set some frequently used sets of sprites
	
	loadSprite("res/sprites/small hill.nbt");
	loadSprite("res/sprites/big hill.nbt");
	loadSprite("res/sprites/small cloud.nbt");
	loadSprite("res/sprites/medium cloud.nbt");
	loadSprite("res/sprites/big cloud.nbt");
	loadSprite("res/sprites/small bush.nbt");
	loadSprite("res/sprites/medium bush.nbt");
	loadSprite("res/sprites/big bush.nbt");

	// Set up layers
	glGenFramebuffers(1, &bgLayerFB);
	glBindFramebuffer(GL_FRAMEBUFFER, bgLayerFB);
	glGenTextures(1, &bgLayer);
	glBindTexture(GL_TEXTURE_2D, bgLayer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 240, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bgLayer, 0);

	glGenFramebuffers(1, &objLayerFB);
	glBindFramebuffer(GL_FRAMEBUFFER, objLayerFB);
	glGenTextures(1, &objLayer);
	glBindTexture(GL_TEXTURE_2D, objLayer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 240, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, objLayer, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	loadMod("SMBase.dll");

	loadMarioStageDefinition("small.nbt");
	loadMarioStageDefinition("dead.nbt");
	player = Mario();
	player.physics_attributes.gravityAffected = true;
	player.physics_attributes.isSolid = false;
	player.pos = glm::vec2(200, 105);
	player.during_tick.push_back(getModProcess<tick_function>("SMBase.dll", "player_tick"));
	player.post_collision.push_back(getModProcess<tick_function>("SMBase.dll", "player_post_collision"));
	player.overrides.render = true;
	player.during_render.push_back(getModProcess<tick_function>("SMBase.dll", "player_render"));
	player.stage.switchTo(getMarioStageDefinition("small mario"));
	player.layer = Object::OBJECT_LAYER;
	tiles.push_back(&player);

	loadSprite("res/sprites/ground.nbt");

	tiles.push_back(new Object());
	tiles[1]->sprites.push_back(Object::ObjectSprite(sprites["ground"], glm::ivec2(0, 0), glm::vec2(256, 24), Object::ObjectSprite::SMB_REPEAT));
	tiles[1]->pos = glm::vec2(0, 200);
	tiles[1]->physics_attributes.hitbox = glm::vec4(0, 0, 32 * 8, 3 * 8);

	loadSprite("res/sprites/mushroom.nbt");
	loadObject("mushroom.nbt");
	createObjectFromDefinition("mushroom", 200, 105);

	tiles.push_back(new Object());
	tiles[3]->sprites.push_back(Object::ObjectSprite(sprites["ground"], glm::ivec2(0), glm::vec2(16, 32), Object::ObjectSprite::SMB_REPEAT));
	tiles[3]->pos = glm::vec2(200, 168);
	tiles[3]->physics_attributes.hitbox = glm::vec4(0, 0, 16, 32);

	tiles.push_back(new Object());
	tiles[4]->sprites.push_back(Object::ObjectSprite(sprites["ground"], glm::ivec2(0)));
	tiles[4]->pos = glm::vec2(80, 184);
	tiles[4]->physics_attributes.hitbox = glm::vec4(0, 0, 16, 16);


	tiles.push_back(new Object());
	tiles[5]->sprites.push_back(Object::ObjectSprite(sprites["ground"], glm::ivec2(0), glm::vec2(48, 16), Object::ObjectSprite::SMB_REPEAT));
	tiles[5]->pos = glm::vec2(80, 136);
	tiles[5]->physics_attributes.hitbox = glm::vec4(0, 0, 48, 16);
	
	palette = palettes[5];

	handler.app = this;
	handler.glfw_window = WINDOW;
	handler.window_width = &WINDOW_WIDTH;
	handler.window_height = &WINDOW_HEIGHT;
	handler.sound_context = context;
	handler.sound_device = device;
	handler.pmouseX = &pmouseX;
	handler.pmouseY = &pmouseY;
	handler.pmouseScroll = &pmouseScroll;
	handler.mouseX = &mouseX;
	handler.mouseY = &mouseY;
	handler.mouseScroll = &mouseScroll;
	handler.pmouseLeft = &pmouseLeft;
	handler.pmouseMiddle = &pmouseMiddle;
	handler.pmouseRight = &pmouseRight;
	handler.mouseLeft = &mouseLeft;
	handler.mouseMiddle = &mouseMiddle;
	handler.mouseRight = &mouseRight;
	handler.keys = keys;
	handler.pKeys = pKeys;
}

unsigned int App::getPalette(unsigned int index) {
	return palettes[index];
}

void App::tick(double delta, double next_delta) {
	
	delta = 1.0 / 60.0;
	next_delta = 1.0 / 60.0;
	
	handler.delta = &delta;
	handler.next_delta = &next_delta;

	if (keys[GLFW_KEY_R]) {
		tiles[2]->pos = glm::vec2(80, 120);
		tiles[2]->facing = RIGHT;
	}

	for (auto a : mods)
		if (GetProcAddress(a.second, "_mod_tick"))
			((mod_function)GetProcAddress(a.second, "_mod_tick"))(handler);

	if (worldOffset.x > player.pos.x) {
		player.pos.x = worldOffset.x;
		player.vel.x = 0;
	}
	else if (player.pos.x - worldOffset.x >= 80 && player.vel.x > 0)
		worldOffset.x += player.vel.x * delta;

	for (Object *o : tiles) {
		tickObject(o, delta, next_delta);
	}


	for (auto a : mods)
		if (GetProcAddress(a.second, "_mod_tick_post"))
			((mod_function)GetProcAddress(a.second, "_mod_tick_post"))(handler);
}

void App::render() {

	for (auto a : mods)
		if (GetProcAddress(a.second, "_mod_render_pre"))
			((mod_function)GetProcAddress(a.second, "_mod_render_pre"))(handler);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	uniMat("screenspace", glm::ortho<float>(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0));

	glUseProgram(tileShader);
	uniBool("check_transparency", true);
	uniInt("sprite", 0);
	uniInt("object_palette", 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tilesetTextureFile);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, palette);

	glBindFramebuffer(GL_FRAMEBUFFER, objLayerFB);
	glViewport(0, 0, NES_WIDTH, NES_HEIGHT);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, bgLayerFB);
	glViewport(0, 0, NES_WIDTH, NES_HEIGHT);
	glClearColor(35.0 / 256.0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	uniMat("screenspace", glm::ortho<float>(0, NES_WIDTH, NES_HEIGHT, 0));

	drawObject(&titlero);
	drawObject(&copyrightro);
	drawSprite(sprites["small cloud"], 0, 0);
	drawSprite(sprites["medium cloud"], 32, 0);
	drawSprite(sprites["big cloud"], 80, 0);
	drawSprite(sprites["small bush"], 0, 184);
	drawSprite(sprites["medium bush"], 32, 184);
	drawSprite(sprites["big bush"], 80, 184);

	for (auto a : mods)
		if (GetProcAddress(a.second, "_mod_render"))
			((mod_function)GetProcAddress(a.second, "_mod_render"))(handler);

	for (int i = 0; i < tiles.size(); i++) {
		
		if (tiles[i]->layer == Object::BACKGROUND)
			glBindFramebuffer(GL_FRAMEBUFFER, bgLayerFB);
		else glBindFramebuffer(GL_FRAMEBUFFER, objLayerFB);

		if(!tiles[i]->overrides.render)
			drawObject(tiles[i]);
		for (auto func : tiles[i]->during_render)
			func(tiles[i], handler);
	}

	for (auto a : mods)
		if (GetProcAddress(a.second, "_mod_render_post"))
			((mod_function)GetProcAddress(a.second, "_mod_render_post"))(handler);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(blitShader);
	uniInt("image", 0);
	uniInt("gamepalette", 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, paletteTextureFile);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bgLayer);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, objLayer);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	for (auto a : mods)
		if (GetProcAddress(a.second, "_mod_render_end"))
			((mod_function)GetProcAddress(a.second, "_mod_render_end"))(handler);
}