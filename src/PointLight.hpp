#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

struct PointLight
{
    glm::vec3 pos = glm::vec3(0.5f);
    glm::vec3 col = glm::vec3(1);
    float affectDistance = 0.3;
    float intensity = 1.0;
    void UploadBuffer(GLuint programID, std::string uniformName)
    {
        glUniform3fv(glGetUniformLocation(programID, (uniformName + ".pos").c_str()), 1, glm::value_ptr(pos));
        glUniform3fv(glGetUniformLocation(programID, (uniformName + ".col").c_str()), 1, glm::value_ptr(col));
        glUniform1f(glGetUniformLocation(programID, (uniformName + ".affectDistance").c_str()), affectDistance);
        glUniform1f(glGetUniformLocation(programID, (uniformName + ".intensity").c_str()), intensity);
    }
};