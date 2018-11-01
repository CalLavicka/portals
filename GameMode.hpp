#pragma once

#include "Mode.hpp"

#include "MeshBuffer.hpp"
#include "GL.hpp"

#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <vector>
#include <random>
#include <list>

#include "manymouse/manymouse.h"
#include "Portal.hpp"
#include "Load.hpp"

// Forward declaration before including level
struct GameMode;
#include "Level.hpp"

// The 'GameMode' mode is the main gameplay mode:

struct GameMode : public Mode {
	GameMode();
	virtual ~GameMode();

	//handle_event is called when new mouse or keyboard events are received:
	// (note that this might be many times per frame or never)
	//The function should return 'true' if it handled the event.
	virtual bool handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) override;

	virtual bool handle_mouse_event(ManyMouseEvent const &evt, glm::uvec2 const &window_size) override;

	//update is called at the start of a new frame, after events are handled:
	virtual void update(float elapsed) override;

	//draw is called after update:
	virtual void draw(glm::uvec2 const &drawable_size) override;

	void load_scene();

	std::mt19937 random_gen;

	Portal players[2];
	float rot_speeds[2] = {0,0};
	float sensitivities[2] = {70.f,70.f};
    std::vector <uint32_t> scores = {50};
    uint32_t level = 0;
	Scene::Camera * camera = nullptr;

	Level *current_level = nullptr;

	std::list<Scene::Object *> foods;
	std::vector<Scene::Object *> pots;

    // teleport the object to the assigned portal
	void teleport(Scene::Transform *object_transform, const uint32_t to_portal, bool update_speed = true);

    void show_pause_menu();
    void show_lose();
    void show_win();

    Scene *scene;
};

extern Load< MeshBuffer > vegetable_meshes;

extern Load< GLuint > vegetable_meshes_for_texture_program;

extern Load< GLuint > vegetable_meshes_for_depth_program;

extern Load< GLuint > white_tex;

extern Load< GLuint > empty_vao;