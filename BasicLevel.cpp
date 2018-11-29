#include "BasicLevel.hpp"
#include "BoundingBox.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

BasicLevel::BasicLevel(GameMode *gm,
        Scene::Object::ProgramInfo const &texture_program_info,
            Scene::Object::ProgramInfo const &bloom_program_info,
            Scene::Object::ProgramInfo const &depth_program_info) : Level(gm) {

	{ // Add the three pots
		for(int i=0; i<3; i++) {
			Scene::Object *obj = gm->scene->new_object(gm->scene->new_transform());
			obj->programs[Scene::Object::ProgramTypeDefault] = texture_program_info;
			obj->programs[Scene::Object::ProgramTypeDefault].textures[0] = *white_tex;

			obj->programs[Scene::Object::ProgramTypeShadow] = depth_program_info;

			MeshBuffer::Mesh const &mesh = vegetable_meshes->lookup("Pot");
			obj->programs[Scene::Object::ProgramTypeDefault].start = mesh.start;
			obj->programs[Scene::Object::ProgramTypeDefault].count = mesh.count;

			obj->programs[Scene::Object::ProgramTypeShadow].start = mesh.start;
			obj->programs[Scene::Object::ProgramTypeShadow].count = mesh.count;
			obj->transform->position = glm::vec3(30.f * i - 30.f,-40.f,0.f);
			obj->transform->scale = glm::vec3(0.3f,0.3f,0.3f);
			obj->transform->rotation = glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f,0.f,0.f));
			obj->transform->boundingbox = new BoundingBox(2.0f, 2.0f);
			obj->transform->boundingbox->update_origin(obj->transform->position, glm::vec2(0.0f, 1.0f));
			gm->pots.push_back(obj);
		}
	}

    this->texture_program_info = texture_program_info;
    this->depth_program_info = depth_program_info;
}

std::string food_names[] = {"Broccoli", "Potato", "Carrot", "Mushroom"};

void BasicLevel::spawn_food() {

	Scene::Object *obj = gm->scene->new_object(gm->scene->new_transform());
	obj->programs[Scene::Object::ProgramTypeDefault] = texture_program_info;
	obj->programs[Scene::Object::ProgramTypeDefault].textures[0] = *white_tex;

	obj->programs[Scene::Object::ProgramTypeShadow] = depth_program_info;

	MeshBuffer::Mesh const &mesh = vegetable_meshes->lookup(food_names[gm->random_gen() % 4]);
	obj->programs[Scene::Object::ProgramTypeDefault].start = mesh.start;
	obj->programs[Scene::Object::ProgramTypeDefault].count = mesh.count;

	obj->programs[Scene::Object::ProgramTypeShadow].start = mesh.start;
	obj->programs[Scene::Object::ProgramTypeShadow].count = mesh.count;
	obj->transform->position = glm::vec3(gm->random_gen() % 100 - 50.f,50.f,0.f);
	obj->transform->rotation = glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f,0.f,0.f));
	obj->transform->boundingbox = new BoundingBox(2.0f, 2.0f);
	obj->transform->boundingbox->update_origin(obj->transform->position, glm::vec2(0.0f, 1.0f));
	gm->foods.push_back(obj);
}

void BasicLevel::update(float elapsed) {
	fruit_timer -= elapsed;
	if(fruit_timer < 0.f) {
		fruit_timer += 5.f;
		spawn_food();
	}
}

bool BasicLevel::collision(Scene::Object *o1, Scene::Object *o2) {
    gm->scores[gm->level]+=10;
    fruit_hit++;
    if(fruit_hit == 20) {
        gm->show_win();
    }

    return true;
}

void BasicLevel::fall_off(Scene::Object *o) {
	gm->scores[gm->level] -= 10;
	if(gm->scores[gm->level] <= 0) {
		gm->show_lose();
	}
}
