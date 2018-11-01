#include "OvenLevel.hpp"

#include "texture_program.hpp"
#include "depth_program.hpp"
#include "data_path.hpp"
#include "Load.hpp"

#include <glm/glm.hpp>

using namespace glm;

Load< MeshBuffer > steak_meshes(LoadTagDefault, [](){
	return new MeshBuffer(data_path("steakLevels.pnct"));
});

Load< GLuint > steak_meshes_for_texture_program(LoadTagDefault, [](){
	return new GLuint(steak_meshes->make_vao_for_program(texture_program->program));
});

Load< GLuint > steak_meshes_for_depth_program(LoadTagDefault, [](){
	return new GLuint(steak_meshes->make_vao_for_program(depth_program->program));
});

OvenLevel::OvenLevel(GameMode *gm, Scene::Object::ProgramInfo const &texture_program_info_,
                            Scene::Object::ProgramInfo const &depth_program_info_) : Level(gm), 
                            texture_program_info(texture_program_info_), depth_program_info(depth_program_info_) {
    texture_program_info.vao = *steak_meshes_for_texture_program;
    
    { // Add steak
		steak = gm->scene->new_object(gm->scene->new_transform());
		steak->programs[Scene::Object::ProgramTypeDefault] = texture_program_info;

		steak->programs[Scene::Object::ProgramTypeShadow] = depth_program_info;

		MeshBuffer::Mesh const &mesh = steak_meshes->lookup("steak");
		steak->programs[Scene::Object::ProgramTypeDefault].start = mesh.start;
		steak->programs[Scene::Object::ProgramTypeDefault].count = mesh.count;

	    steak->transform->position = glm::vec3(0.f,10.f,0.f);
	    steak->transform->rotation = glm::angleAxis(glm::radians(-90.f), glm::vec3(0.f,1.f,0.f));
	    steak->transform->boundingbox = new BoundingBox(2.0f, 2.0f);
	    steak->transform->boundingbox->update_origin(steak->transform->position, glm::vec2(0.0f, 1.0f));
	    gm->foods.push_back(steak);

        printf("HELLO: %d\n", mesh.start);
	}
	
}

void OvenLevel::update(float elapsed) {
    // calculate heat
    printf("%f %f\n", steak->transform->position.x, steak->transform->position.y);

    if (abs(steak->transform->position.y) > 20) {
        heat += 30.f * elapsed;
        if (heat > 100.f) {
            gm->show_lose();
        }
    } else {
        heat = max(heat - 30.f * elapsed, 0.f);
        score_timer -= elapsed;
        if (score_timer < 0.f) {
            gm->scores[gm->level]+=1;
            score_timer += 1.f;
        }
    }

    time -= elapsed;
    if (time <= 0.f) {
        gm->show_win();
    }
}

void OvenLevel::fall_off(Scene::Object *o) {
    // lose game
    gm->show_lose();
}

void OvenLevel::render_pass() {
    // render heat
}