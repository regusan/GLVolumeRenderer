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
public:
    std::vector<Vertex> vertices;
    GLuint vao, vbo;
    PointCloud(/* args */);
    PointCloud(const Volume &volume);
    ~PointCloud();

    void UploadBuffer();
    void Draw();

    static std::vector<Vertex> VolumeToVertices(const Volume::VolumeData &data);
};
