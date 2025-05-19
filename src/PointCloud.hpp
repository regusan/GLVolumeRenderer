#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>
#include "Volume.hpp"

struct Vertex
{
    glm::vec3 position;
    glm::vec4 color;
    glm::uint8_t groupe;
};

// 点群クラス
class PointCloud
{
private:
public:
    std::vector<Vertex> vertices;
    GLuint vao, vbo;
    PointCloud(/* args */);
    PointCloud(const Volume &volume);
    ~PointCloud();

    void UploadBuffer();
    void Draw()
    {
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glPointSize(1.0f);
        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, this->vertices.size());
        glBindVertexArray(0);
    }

    static std::vector<Vertex> VolumeToVertices(const Volume::VolumeData &data);
};
