#include "BasicLevel.hpp"
#include "BoundingBox.hpp"
#include "Load.hpp"
#include "data_path.hpp"

#include "draw_text.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

std::string food_names[] = {"Broccoli", "Potato", "Carrot", "Mushroom"};

//Credit:
// THE HAPPY SONG by Nicolai Heidlas Music https://soundcloud.com/nicolai-heidlas
// Creative Commons — Attribution 3.0 Unported— CC BY 3.0
// http://creativecommons.org/licenses/b...
// Music promoted by Audio Library https://youtu.be/cGuaRsXLScQ
Load< Sound::Sample > basic_bgm(LoadTagDefault, [](){
	return new Sound::Sample(data_path("sound_effects/the_happy_song_full.wav"));
});

BasicLevel::BasicLevel(GameMode *gm, Scene::Object::ProgramInfo const &texture_program_info,
                            Scene::Object::ProgramInfo const &depth_program_info) : Level(gm) {


    this->texture_program_info = texture_program_info;
    this->depth_program_info = depth_program_info;

	{ // Add the four pots
		for(int i=0; i<4; i++) {
			Scene::Object *obj = gm->scene->new_object(gm->scene->new_transform());
			obj->programs[Scene::Object::ProgramTypeDefault] = texture_program_info;
			obj->programs[Scene::Object::ProgramTypeDefault].textures[0] = *white_tex;

			obj->programs[Scene::Object::ProgramTypeShadow] = depth_program_info;

			MeshBuffer::Mesh const &mesh = vegetable_meshes->lookup("Pot");
			obj->programs[Scene::Object::ProgramTypeDefault].start = mesh.start;
			obj->programs[Scene::Object::ProgramTypeDefault].count = mesh.count;

			obj->programs[Scene::Object::ProgramTypeShadow].start = mesh.start;
			obj->programs[Scene::Object::ProgramTypeShadow].count = mesh.count;
			obj->transform->position = glm::vec3(30.f * i - 45.f,-40.f,0.f);
			obj->transform->scale = glm::vec3(0.3f,0.3f,0.3f);
			obj->transform->rotation = glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f,0.f,0.f));
			obj->transform->boundingbox = new BoundingBox(2.0f, 2.0f);
			obj->transform->boundingbox->update_origin(obj->transform->position, glm::vec2(0.0f, 1.0f));
			gm->pots.push_back(obj);
			obj->data = food_names[i];

			Scene::Object *food = create_food(food_names[i]);
			food->transform->position = obj->transform->position;
			food->transform->position.z = 10.f;
		}
	}

	messagetime = 5.f;

	bgm = basic_bgm->play(gm->camera->transform->position, 1.0f, Sound::Loop);  // play bgm
}

BasicLevel::~BasicLevel() {
	if (this->bgm) this->bgm->stop();  // stop bgm
}

Scene::Object *BasicLevel::create_food(std::string veg_name) {
	Scene::Object *obj = gm->scene->new_object(gm->scene->new_transform());
	obj->programs[Scene::Object::ProgramTypeDefault] = texture_program_info;
	obj->programs[Scene::Object::ProgramTypeDefault].textures[0] = *white_tex;

	obj->programs[Scene::Object::ProgramTypeShadow] = depth_program_info;

	MeshBuffer::Mesh const &mesh = vegetable_meshes->lookup(veg_name);
	obj->programs[Scene::Object::ProgramTypeDefault].start = mesh.start;
	obj->programs[Scene::Object::ProgramTypeDefault].count = mesh.count;

	obj->programs[Scene::Object::ProgramTypeShadow].start = mesh.start;
	obj->programs[Scene::Object::ProgramTypeShadow].count = mesh.count;
	obj->transform->rotation = glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f,0.f,0.f));

	return obj;
}

void BasicLevel::spawn_food() {

	uint32_t idx = gm->random_gen() % 4;
	Scene::Object *obj = create_food(food_names[idx]);
	obj->transform->position = glm::vec3(gm->random_gen() % 150 - 75.f,50.f,0.f);
	obj->transform->boundingbox = new BoundingBox(2.0f, 2.0f);
	obj->transform->boundingbox->update_origin(obj->transform->position, glm::vec2(0.0f, 1.0f));
	obj->data = food_names[idx];
	gm->foods.push_back(obj);
}

void BasicLevel::update(float elapsed) {
	fruit_timer -= elapsed;
	if(fruit_timer < 0.f) {
		fruit_timer += 5.f;
		spawn_food();
	}

	messagetime -= elapsed;
}

bool BasicLevel::collision(Scene::Object *o1, Scene::Object *o2) {
	if (o1->data == o2->data) {
		gm->scores[gm->level]+=10;
		fruit_hit++;
		if(fruit_hit == 20) {
			gm->show_win();
		}
	} else {
		gm->scores[gm->level] -= 10;
		if(gm->scores[gm->level] <= 0) {
			gm->show_lose();
		}
	}

    return true;
}

void BasicLevel::fall_off(Scene::Object *o) {
	if(gm->scores[gm->level] >= 10) gm->scores[gm->level] -= 10;
    else if(gm->scores[gm->level] > 0) gm->scores[gm->level] = 0;
	if(gm->scores[gm->level] == 0) {
		gm->show_lose();
	}
}

void BasicLevel::render_pass() {

	glDisable(GL_DEPTH_TEST);

	if (messagetime > 0.f) {
        std::string messages[] = {"GET THE VEGETABLES", "INTO THE", "CORRECT POTS"};
        float height = 0.15f;

        float ypos = -0.3f;
        for(std::string message : messages) {
            float width = text_width(message, height);
            draw_text(message, glm::vec2( -width/2.f, ypos), height,
                glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            ypos -= 0.2f;
        }
    }

	glEnable(GL_DEPTH_TEST);
}
