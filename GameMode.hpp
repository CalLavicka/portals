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
	void spawn_food();

	std::mt19937 random_gen;
	float camera_spin = 0.0f;
	float spot_spin = 0.0f;
	float fruit_timer = 5.f;

	Portal players[2];
	float rot_speeds[2] = {0,0};
	float sensitivities[2] = {50.f,50.f};
    std::vector <uint32_t> scores = {50};
    uint32_t level = 0;

	std::list<Scene::Object *> foods;

	std::vector<Scene::Object *> pots;

	void teleport(Scene::Transform *object_transform, const uint32_t to_portal);

    void show_pause_menu();
    void show_lose();
    void show_win();

    Scene *scene;
};
