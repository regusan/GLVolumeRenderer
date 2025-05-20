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

public:
    struct Cell
    {
        char intencity;
        unsigned char id;
    };

    using VolumeData = std::vector<std::vector<std::vector<Cell>>>;
    size_t size;
    VolumeData data;
    Volume(std::ifstream &file);
    std::string Sammary();
    static void Clustering(VolumeData &v);
    void Draw();
    void UploadBuffer();
    GLuint volumeTexture = 0;
    GLuint screenQuadVAO = 0;
};

std::ostream &operator<<(std::ostream &os, Volume &v);