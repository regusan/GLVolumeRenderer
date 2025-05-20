#pragma once

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

// vec2
inline std::ostream &operator<<(std::ostream &os, const glm::vec2 &v)
{
    os << "(" << v.x << ", " << v.y << ")";
    return os;
}

// vec3
inline std::ostream &operator<<(std::ostream &os, const glm::vec3 &v)
{
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}

// vec4
inline std::ostream &operator<<(std::ostream &os, const glm::vec4 &v)
{
    os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
    return os;
}