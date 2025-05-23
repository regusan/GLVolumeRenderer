
#include <fstream>
#include <sstream>
#include <chrono>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "PointCloud.hpp"
#include "Volume.hpp"

using namespace std;

PointCloud::PointCloud(const Volume &volume)
{
    this->vertices = PointCloud::VolumeToVertices(volume.data);
}

PointCloud::PointCloud(/* args */)
{
}

PointCloud::~PointCloud()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

std::vector<Vertex> PointCloud::VolumeToVertices(const Volume::VolumeData &data)
{
    std::vector<Vertex> vertices;
    vertices.reserve(data.size() * data.size() * data.size() * 0.1); // 10%でとりあえずアロケート
    int N = data.size();
    float scale = 1.0f / static_cast<float>(N);
    constexpr size_t step = 1;
    for (int i = 0; i < N; i += step)
    {
        for (int j = 0; j < N; j += step)
        {
            for (int k = 0; k < N; k += step)
            {
                auto cell = &data[i][j][k];
                if (data[i][j][k].intencity != 0)
                {
                    float x = (i + 0.5f) * scale - 0.5f;
                    float y = (j + 0.5f) * scale - 0.5f;
                    float z = (k + 0.5f) * scale - 0.5f;
                    float colorValue = static_cast<float>(cell->intencity) / 255.0f;
                    vertices.push_back((Vertex){
                        glm::vec3(x, y, z),
                        glm::float32(colorValue),
                    });
                    // cell->id});
                }
            }
        }
    }

    return vertices;
}

void PointCloud::UploadBuffer()
{
    glGenVertexArrays(1, &this->vao);
    glGenBuffers(1, &this->vbo);

    glBindVertexArray(this->vao);
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
    glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), this->vertices.data(), GL_STATIC_DRAW);

    // 頂点座標（位置）の設定
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, position)));
    glEnableVertexAttribArray(0);

    /*     // 頂点カラーの設定
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, color)));
        glEnableVertexAttribArray(1); */

    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, alpha)));
    glEnableVertexAttribArray(1);

    /*     // 頂点グループの設定
        glVertexAttribPointer(2, 1, GL_BYTE, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, groupe)));
        glEnableVertexAttribArray(2); */

    glBindVertexArray(0);
}

void PointCloud::Draw()
{
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(1.0f);
    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, this->vertices.size());
    glBindVertexArray(0);
}