#include "OvenLevel.hpp"

#include "texture_program.hpp"
#include "depth_program.hpp"
#include "data_path.hpp"
#include "Load.hpp"
#include "compile_program.hpp"
#include "gl_errors.hpp"
#include "draw_text.hpp"

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

Load< Sound::Sample > oven_bgm(LoadTagDefault, [](){
	return new Sound::Sample(data_path("sound_effects/taiko_warrior_trimmed.wav"));
});

Load< Sound::Sample > oven_prelude(LoadTagDefault, [](){
	return new Sound::Sample(data_path("sound_effects/taiko_initial_pipe.wav"));
});

Load< GLuint > heat_program(LoadTagDefault, [](){
	GLuint program = compile_program(
		//this draws a triangle that covers the entire screen:
		"#version 330\n"
		//"uniform float top;\n"
        //"uniform float bottom;\n"
		"uniform mat4 cam_scale;\n"
        "out vec2 position;\n"
		"void main() {\n"
		//"	if (gl_VertexID < 4) {\n"
		//"		gl_Position = cam_scale * vec4(100 * (2 * (gl_VertexID & 1) - 1), 100 * (gl_VertexID & 2) + top, 0.5, 1.0);\n"
		//"	} else if(gl_VertexID>=4){\n"
		//"		gl_Position = cam_scale * vec4(100 * (2 * (gl_VertexID & 1) - 1), -100 * (gl_VertexID & 2) + bottom, 0.5, 1.0);\n"
		//"	}\n"
        "   position = vec2(100 * (2 * (gl_VertexID & 1) - 1),  100 * (gl_VertexID & 2) - 100);\n"
        "   gl_Position = cam_scale * vec4(position, 0.5, 1.0);\n"
		"}\n"
		,

		"#version 330\n"
        "in vec2 position;\n"
        "uniform float top;\n"
        "uniform float bottom;\n"
        "uniform float time;\n"
		"out vec4 fragColor;\n"
		"void main() {\n"
        //"   vec2 position = gl_FragCoord.xy;\n"
        "   float mod = sin(cos(time) * 2 + position.x * 0.5f) * cos(time);\n"
        "   if (position.y > top + mod || position.y < bottom + mod) {\n"
        "       float distortion = sin(time * 4 + position.y * 0.5f);\n"
        "       float color = cos(distortion + position.x * 0.2f);\n"
		"	    fragColor = vec4(0.9f + color * 0.1f, 0.1f - color * 0.05f, 0.0, 0.2);\n"
        "   } else {\n"
        "       fragColor = vec4(0.0);\n"
        "   }\n"
		"}\n"
	);

	return new GLuint(program);
});

Load< GLuint > meat1_tex(LoadTagDefault, [](){
        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glm::u8vec4 meat(0xa0, 0xa0, 0xa0, 0xff);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, glm::value_ptr(meat));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        return new GLuint(tex);
        });

Load< GLuint > meat2_tex(LoadTagDefault, [](){
        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glm::u8vec4 meat(0x40, 0x40, 0x40, 0xff);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, glm::value_ptr(meat));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        return new GLuint(tex);
        });

Load< GLuint > meat3_tex(LoadTagDefault, [](){
        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glm::u8vec4 meat(0x25, 0x23, 0x23, 0xff);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, glm::value_ptr(meat));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        return new GLuint(tex);
        });

Load< GLuint > meat4_tex(LoadTagDefault, [](){
        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glm::u8vec4 meat(0x11, 0x05, 0x05, 0xff);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, glm::value_ptr(meat));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        return new GLuint(tex);
        });

OvenLevel::OvenLevel(GameMode *gm,
                    Scene::Object::ProgramInfo const &texture_program_info_,
                    Scene::Object::ProgramInfo const &depth_program_info_) : Level(gm),
                    texture_program_info(texture_program_info_),
                    depth_program_info(depth_program_info_) {
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
        steak->transform->scale = glm::vec3(2.0f,2.0f,2.0f);
	    steak->transform->boundingbox = new BoundingBox(4.0f, 4.0f);
	    steak->transform->boundingbox->update_origin(steak->transform->position, glm::vec2(0.0f, 1.0f));
	    gm->foods.push_back(steak);

        //printf("HELLO: %d\n", mesh.start);
	}

    Scene::Object::ProgramInfo oven_program_info =
        texture_program_info;
    oven_program_info.set_uniforms = [](){
        glUniform1f(texture_program->glow_amt_float, 0.0f);
        glUniform3fv(texture_program->sky_color_vec3, 1,
                glm::value_ptr(glm::vec3(0.7f,0.6f,0.6f)));
        glUniform3fv(texture_program->sky_direction_vec3, 1,
                glm::value_ptr(glm::vec3(-0.6f, 0.5f, 1.0f)));


    };

    { // Add oven
		Scene::Object * oven = gm->scene->new_object(gm->scene->new_transform());
		oven->programs[Scene::Object::ProgramTypeDefault] = oven_program_info;
            //texture_program_info;

		oven->programs[Scene::Object::ProgramTypeShadow] = depth_program_info;

		MeshBuffer::Mesh const &mesh = steak_meshes->lookup("oven");
		oven->programs[Scene::Object::ProgramTypeDefault].start = mesh.start;
		oven->programs[Scene::Object::ProgramTypeDefault].count = mesh.count;
	    oven->transform->rotation = glm::angleAxis(glm::radians(-90.f), glm::vec3(0.f,0.f,1.f))
                                        * glm::angleAxis(glm::radians(-90.f), glm::vec3(0.f,1.f,0.f));
        oven->transform->scale = vec3(3.f, 7.5f, 8.f);
        oven->transform->position = vec3(0,7.0f,0.0f);
    }

    messagetime = 5.f;

	oven_prelude->play(gm->camera->transform->position, 1.0f);  // play bgm
}

void OvenLevel::update(float elapsed) {
    // calculate heat
    //printf("%f %f\n", steak->transform->position.x, steak->transform->position.y);
    if(heat > 10.0f){
        if(heat>80.0f){
            steak->programs[Scene::Object::ProgramTypeDefault].textures[0] =
                *meat4_tex;
        }else if(heat>60.0f){
            steak->programs[Scene::Object::ProgramTypeDefault].textures[0] =
                *meat3_tex;
        }else if(heat>40.0f){
            steak->programs[Scene::Object::ProgramTypeDefault].textures[0] =
                *meat2_tex;
        }else{
            steak->programs[Scene::Object::ProgramTypeDefault].textures[0] =
                *meat1_tex;
        }
    }else {
        steak->programs[Scene::Object::ProgramTypeDefault].textures[0] =
                *white_tex;
    }


    if (steak->transform->position.y > top || steak->transform->position.y < bottom) {
        heat += 10.f * elapsed;
        if (heat > 100.f) {
            gm->show_lose();
        }
    } else {
        //heat = max(heat - 20.f * elapsed, 0.f);
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

    top = sin((60.f - time) * 0.2f) * 30.f + 20.f;
    bottom = top - 40.f;

    messagetime -= elapsed;

    if (prelude_countdown > 0.0f) {
        prelude_countdown -= elapsed;
        if (prelude_countdown <= 0.0f) {
            bgm = oven_bgm->play(gm->camera->transform->position, 1.0f, Sound::Loop);  // play bgm
        }
    }
}

void OvenLevel::fall_off(Scene::Object *o) {
    // lose game
    gm->show_lose();
}

void OvenLevel::render_pass() {
    // render heat
    glUseProgram(*heat_program);
	glBindVertexArray(*empty_vao);

    glDisable(GL_DEPTH_TEST);

    static GLuint uniform_top = glGetUniformLocation(*heat_program, "top");
    static GLuint uniform_bottom = glGetUniformLocation(*heat_program, "bottom");
	static GLuint cam_scale_mat4 = glGetUniformLocation(*heat_program, "cam_scale");
    static GLuint uniform_time = glGetUniformLocation(*heat_program, "time");


	glm::mat4 cam_scale = gm->camera->make_projection() * gm->camera->transform->make_world_to_local();

    glUniform1f(uniform_top, top);
    glUniform1f(uniform_bottom, bottom);
    glUniform1f(uniform_time, time);
	glUniformMatrix4fv(cam_scale_mat4, 1, GL_FALSE, glm::value_ptr(cam_scale));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	//glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);

    if (messagetime > 0.f) {
        std::string messages[] = {"DONT BURN", "THE MEAT", "FOR 60 SECONDS"};
        float height = 0.1f;

        float ypos = -0.3f;
        for(std::string message : messages) {
            float width = text_width(message, height);
            draw_text(message, glm::vec2( -width/2.f, ypos), height,
                glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            ypos -= 0.2f;
        }
    }

    glEnable(GL_DEPTH_TEST);

    GL_ERRORS();

}
