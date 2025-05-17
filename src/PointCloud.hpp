#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>

class Volume;

struct Vertex
{
    glm::vec3 position;
    glm::vec4 color;
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

    static std::vector<Vertex> VolumeToVertices(const std::vector<std::vector<std::vector<char>>> &data);
};
