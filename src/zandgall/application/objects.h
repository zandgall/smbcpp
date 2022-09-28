#pragma once
#ifndef OBJECTS_H
#define OBJECTS_H
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <map>
#include "App.h"
#include <nbt/nbt.hpp>
#ifdef SMBCPP
#include "../../../include/mod_handler.h"
#else
struct ModHandler;
#endif
#define LEFT -1
#define RIGHT 1

struct Sprite {
	int width = 0, height = 0;
	glm::ivec2* tiles = nullptr; // width*height length array of coordinates to a tile
	unsigned int palette = 0;
	bool* flipmap = nullptr; // width * height length array determining whether or not a 8x8 tile should be flipped when drawn
};

struct ivec2comp final {
	constexpr bool operator()(const glm::ivec2& a, const glm::ivec2& b) const noexcept {
		if (a.x == b.x)
			return a.y <= b.y;
		else return a.x <= b.x;
	}
};
class App;
struct Object;
struct Mario;
typedef void (*tick_function)(Object*, ModHandler&);
typedef void (*interaction_function)(Object*, Object*, ModHandler&);
typedef void (*mario_interaction_function)(Object*, Mario*, ModHandler&);
struct Object {
	enum {
		BACKGROUND, OBJECT_LAYER
	};
	glm::vec2 pos = glm::vec2(0), vel = glm::vec2(0), force = glm::vec2(0); // world position unless isStatic = true
	struct ObjectSprite {
		enum {
			SMB_TRANSPARENT, SMB_EDGE_PIXEL, SMB_EDGE_TILE, SMB_REPEAT
		};
		Sprite* sprite = nullptr;
		glm::ivec2 offset = glm::ivec2(0), size = glm::ivec2(16);
		unsigned int edge_continue_mode = SMB_TRANSPARENT;
		ObjectSprite() {}
		ObjectSprite(Sprite* sprite, glm::ivec2 off, glm::ivec2 size = glm::ivec2(16), unsigned int edge_continue_mode = SMB_TRANSPARENT) {
			this->sprite = sprite;
			offset = off;
			this->size = size;
			this->edge_continue_mode = edge_continue_mode;
		}
		ObjectSprite(const ObjectSprite& o) {
			sprite = o.sprite;
			offset = o.offset;
			size = o.size;
			edge_continue_mode = o.edge_continue_mode;
		};
	};
	unsigned int layer = BACKGROUND;
	std::vector<ObjectSprite> sprites = std::vector<ObjectSprite>();
	struct PhysicsAttributes {
		bool isStatic = false;
		bool isSolid = true;
		bool gravityAffected = false;
		glm::vec4 hitbox = glm::vec4(0);
		glm::bvec4 collisions = glm::bvec4(0);

		PhysicsAttributes() {
			isStatic = false;
			isSolid = true;
			gravityAffected = false;
		};
		// Copy constructor
		PhysicsAttributes(const PhysicsAttributes& p) {
			isStatic = p.isStatic;
			isSolid = p.isSolid;
			gravityAffected = p.gravityAffected;
			hitbox = p.hitbox;
			collisions = p.collisions;
		};
	} physics_attributes = PhysicsAttributes();
	struct ObjectType {
		std::string name = "tile";

		ObjectType() {};
		// Copy constructor
		ObjectType(const ObjectType& o) {
			name = o.name;
		};
	} type = ObjectType();
	int8_t facing = RIGHT;

	struct Overrides {
		bool tick = false;
		bool collision = false;
		bool render = false;
		Overrides() {};
		// Copy constructor
		Overrides(const Overrides& o) {
			tick = o.tick;
			collision = o.collision;
			render = o.render;
		};
	} overrides;

	std::vector<tick_function> during_tick = std::vector<tick_function>();
	std::vector<tick_function> post_collision = std::vector<tick_function>();
	//tick_function post_collision = nullptr;

	std::vector < mario_interaction_function> touches_mario = std::vector <mario_interaction_function>();
	std::vector < interaction_function> touches_object = std::vector <interaction_function>();

	std::vector <interaction_function> touches_floor = std::vector <interaction_function>();
	std::vector <interaction_function> touches_ceil = std::vector <interaction_function>();
	std::vector <interaction_function> touches_wall = std::vector <interaction_function>();
	std::vector <interaction_function> touches_wall_left = std::vector <interaction_function>();
	std::vector <interaction_function> touches_wall_right = std::vector <interaction_function>();

	std::vector<tick_function> during_render = std::vector<tick_function>();

	Object() {
		pos = glm::vec2(0);
		vel = glm::vec2(0);
		force = glm::vec2(0);

		physics_attributes = PhysicsAttributes();
		type = ObjectType();
		sprites = std::vector<ObjectSprite>();

		during_tick = std::vector<tick_function>();
		post_collision = std::vector<tick_function>();

		touches_mario = std::vector <mario_interaction_function>();
		touches_object = std::vector <interaction_function>();

		touches_floor = std::vector <interaction_function>();
		touches_ceil = std::vector <interaction_function>();
		touches_wall = std::vector <interaction_function>();
		touches_wall_left = std::vector <interaction_function>();
		touches_wall_right = std::vector <interaction_function>();
	};
	// Copy constructor
	Object(const Object& o) {
		pos = o.pos;
		vel = o.vel;
		force = o.force;
		sprites = o.sprites;
		physics_attributes = PhysicsAttributes(o.physics_attributes);
		type = ObjectType(o.type);
		layer = o.layer;
		facing = o.facing;
		during_tick = o.during_tick;
		post_collision = o.post_collision;
		touches_mario = o.touches_mario;
		touches_object = o.touches_object;
		touches_floor = o.touches_floor;
		touches_ceil = o.touches_ceil;
		touches_wall = o.touches_wall;
		touches_wall_left = o.touches_wall_left;
		touches_wall_right = o.touches_wall_right;
	};
};

typedef int (*animation_index_grabber)(Mario*, ModHandler&);
typedef void (*mario_function)(Mario*, ModHandler&);

struct Mario : Object {
	struct Animation {
		std::vector<Sprite*> sprites = std::vector<Sprite*>();
		// Length of animation in seconds, not frames
		double animation_length = 1;
		mario_function custom_timing_func = nullptr;
	};
	struct Stage {
		glm::vec4 hitbox = glm::vec4(3, 3, 11, 13);
		bool can_shoot_fireballs = false;
		int palette = 0;
		std::string name = "small";
		nbt::compound custom_data;
		const Stage* on_damage, *on_powerup;
		mario_function on_damage_func = nullptr, on_powerup_func = nullptr;
		std::vector<Animation> animations = std::vector<Animation>();

		// A function that tells the application what animation from 'animations' the owner mario object should use to render
		animation_index_grabber current_animation = nullptr;
		void switchTo(const Stage* next) {
			hitbox = next->hitbox;
			can_shoot_fireballs = next->can_shoot_fireballs;
			palette = next->palette;
			name = next->name;
			custom_data = next->custom_data;
			on_damage = next->on_damage;
			on_powerup = next->on_powerup;
			on_damage_func = next->on_damage_func;
			on_powerup_func = next->on_powerup_func;
			animations = next->animations;
			current_animation = next->current_animation;
		}
		Stage() {}
		//Stage(const Stage& s) {
		//	hitbox = s.hitbox;

		//}
	} stage;
	//unsigned int* texture;
	bool jumping = false, star = false;
	double frame_index = 0;
	// GL_RG8 texture that's used to grab a color from the palette texture
	Mario() {};
};

#define AABB(a, b) ((a).x <= (b).x + (b).z && (a).x + (a).z >= (b).x && (a).y <= (b).y + (b).w && (a).y + (a).w >= (b).y)
#endif