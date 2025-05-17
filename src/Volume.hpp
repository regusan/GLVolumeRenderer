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
    size_t size;
    std::vector<std::vector<std::vector<char>>> data;
    Volume(std::ifstream &file);
};
