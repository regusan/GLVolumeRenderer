#pragma once
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

/// @brief 3Dボリュームクラス
class Volume
{
private:
    glm::vec3 CalcNormalAtIndex(size_t x, size_t y, size_t z);

public:
    struct Cell
    {
        char intencity;
        unsigned char id;
        // glm::vec3 normal;
    };

    using VolumeData = std::vector<std::vector<std::vector<Cell>>>;
    size_t size;
    VolumeData data;
    Volume(std::ifstream &file);
    std::string Sammary();
    static void Clustering(VolumeData &v);
    void Draw();
    void UploadBuffer();
    void CalcNormal();
    GLuint volumeTexture = 0;
    GLuint cubeVAO = 0;
};

std::ostream &operator<<(std::ostream &os, Volume &v);