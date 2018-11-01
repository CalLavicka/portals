#include "OvenLevel.hpp"

#include "texture_program.hpp"
#include "depth_program.hpp"
#include "data_path.hpp"
#include "Load.hpp"
#include "compile_program.hpp"
#include "gl_errors.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

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

Load< GLuint > heat_program(LoadTagDefault, [](){
	GLuint program = compile_program(
		//this draws a triangle that covers the entire screen:
		"#version 330\n"
		"uniform float top;\n"
        "uniform float bottom;\n"
		"uniform mat4 cam_scale;\n"
		"void main() {\n"
		"	if (gl_VertexID < 4) {\n"
		"		gl_Position = cam_scale * vec4(100 * (2 * (gl_VertexID & 1) - 1), 100 * (gl_VertexID & 2) + top, -1.0, 1.0);\n"
		"	} else {\n"
		"		gl_Position = cam_scale * vec4(100 * (2 * (gl_VertexID & 1) - 1), -100 * (gl_VertexID & 2) + bottom, -1.0, 1.0);\n"
		"	}\n"
		"}\n"
		,

		"#version 330\n"
		"out vec4 fragColor;\n"
		"void main() {\n"
		"	fragColor = vec4(1.0, 0.1, 0.1, 0.2);\n"
		"}\n"
	);

	return new GLuint(program);
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

        //printf("HELLO: %d\n", mesh.start);
	}

}

void OvenLevel::update(float elapsed) {
    // calculate heat
    //printf("%f %f\n", steak->transform->position.x, steak->transform->position.y);

    if (steak->transform->position.y > top || steak->transform->position.y < bottom) {
        heat += 40.f * elapsed;
        if (heat > 100.f) {
            gm->show_lose();
        }
    } else {
        heat = max(heat - 20.f * elapsed, 0.f);
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

    top = sin((120.f - time) * 0.2f) * 30.f + 20.f;
    bottom = top - 40.f;
}

void OvenLevel::fall_off(Scene::Object *o) {
    // lose game
    gm->show_lose();
}

void OvenLevel::render_pass() {
    // render heat
    glUseProgram(*heat_program);
	glBindVertexArray(*empty_vao);

    glClear(GL_DEPTH_BUFFER_BIT);

    static GLuint uniform_top = glGetUniformLocation(*heat_program, "top");
    static GLuint uniform_bottom = glGetUniformLocation(*heat_program, "bottom");
	static GLuint cam_scale_mat4 = glGetUniformLocation(*heat_program, "cam_scale");


	glm::mat4 cam_scale = gm->camera->make_projection() * gm->camera->transform->make_world_to_local();

    glUniform1f(uniform_top, top);
    glUniform1f(uniform_bottom, bottom);
	glUniformMatrix4fv(cam_scale_mat4, 1, GL_FALSE, glm::value_ptr(cam_scale));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);

    GL_ERRORS();

}
