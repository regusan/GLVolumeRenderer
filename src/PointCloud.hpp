#pragma once
#include <vector>
#include "Volume.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

struct Vertex
{
    glm::vec3 position;
    glm::float32 alpha;
    // glm::uint8_t groupe;
};

// 点群クラス
class PointCloud
{
private:
    static std::vector<GLuint> ReorderIndices(const std::vector<GLuint> &X, const std::vector<GLuint> &Y, const std::vector<GLuint> &Z, const glm::mat4 &MV);

public:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indicesX;
    std::vector<GLuint> indicesY;
    std::vector<GLuint> indicesZ;

    GLuint vao = 0, vbo = 0, ibo = 0;
    PointCloud(/* args */);
    PointCloud(const Volume &volume);
    ~PointCloud();

    void UploadBuffer();
    void Draw(const glm::mat4 &view);

    static std::vector<Vertex> VolumeToVertices(const Volume::VolumeData &data);
};
