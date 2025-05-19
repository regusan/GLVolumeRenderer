
#include <fstream>
#include <sstream>
#include <chrono>
#include <iostream>

#include "PointCloud.hpp"
#include "Volume.hpp"

using namespace std;

PointCloud::PointCloud(const Volume &volume)
{
    this->vertices = PointCloud::VolumeToVertices(volume.data);
    UploadBuffer();
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
    int N = data.size();
    float scale = 1.0f / static_cast<float>(N);

    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            for (int k = 0; k < N; ++k)
            {
                auto cell = &data[i][j][k];
                if (data[i][j][k].intencity != 0)
                {
                    float x = (i + 0.5f) * scale - 0.5f;
                    float y = (j + 0.5f) * scale - 0.5f;
                    float z = (k + 0.5f) * scale - 0.5f;
                    float colorValue = static_cast<float>(cell->intencity) / 255.0f;
                    vertices.push_back((Vertex){glm::vec3(x, y, z),
                                                glm::vec4(colorValue, colorValue, colorValue, colorValue),
                                                cell->id});
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

    // 頂点カラーの設定
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, color)));
    glEnableVertexAttribArray(1);

    // 頂点グループの設定
    glVertexAttribPointer(2, 1, GL_BYTE, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, groupe)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}