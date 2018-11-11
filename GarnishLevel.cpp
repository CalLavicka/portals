#include "GarnishLevel.hpp"

#include "texture_program.hpp"
#include "depth_program.hpp"
#include "data_path.hpp"
#include "Load.hpp"
#include "compile_program.hpp"
#include "gl_errors.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

Load< MeshBuffer > garnish_meshes(LoadTagDefault, [](){
	return new MeshBuffer(data_path("steakLevels.pnct"));
});

Load< GLuint > garnish_meshes_for_texture_program(LoadTagDefault, [](){
	return new GLuint(garnish_meshes->make_vao_for_program(texture_program->program));
});

Load< GLuint > garnish_meshes_for_depth_program(LoadTagDefault, [](){
	return new GLuint(garnish_meshes->make_vao_for_program(depth_program->program));
});


GarnishLevel::GarnishLevel(GameMode *gm, Scene::Object::ProgramInfo const &texture_program_info_,
                            Scene::Object::ProgramInfo const &depth_program_info_) : Level(gm),
                            texture_program_info(texture_program_info_), depth_program_info(depth_program_info_) {
    texture_program_info.vao = *garnish_meshes_for_texture_program;

    { // Add steak
		steak = gm->scene->new_object(gm->scene->new_transform());
		steak->programs[Scene::Object::ProgramTypeDefault] = texture_program_info;

		steak->programs[Scene::Object::ProgramTypeShadow] = depth_program_info;

		MeshBuffer::Mesh const &mesh = garnish_meshes->lookup("steak");
		steak->programs[Scene::Object::ProgramTypeDefault].start = mesh.start;
		steak->programs[Scene::Object::ProgramTypeDefault].count = mesh.count;

	    steak->transform->position = glm::vec3(0.f,10.f,0.f);
	    steak->transform->rotation = glm::angleAxis(glm::radians(-90.f), glm::vec3(0.f,1.f,0.f));
	    steak->transform->boundingbox = new BoundingBox(2.0f, 2.0f);
	    steak->transform->boundingbox->update_origin(steak->transform->position, glm::vec2(0.0f, 1.0f));
	    gm->foods.push_back(steak);
	}

}

void GarnishLevel::update(float elapsed) {

    time -= elapsed;
    if (time <= 0.f) {
        gm->show_win();
    }

}


