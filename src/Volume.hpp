#pragma once
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

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
    static void Clustering(VolumeData &v);
};

std::ostream &operator<<(std::ostream &os, Volume &v);