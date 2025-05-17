#include "Volume.hpp"

Volume::Volume(std::ifstream &file)
{
    file.seekg(0, std::ios::end);            // 末尾まで移動
    std::streamsize fileSize = file.tellg(); // ファイルサイズ取得
    file.seekg(0, std::ios::beg);            // 先頭まで戻す

    size_t totalElements = static_cast<int>(fileSize);
    this->size = std::cbrt(totalElements); // Nを推測

    if (this->size * this->size * this->size != totalElements)
    {
        std::cerr << "[ERROR] File size is not a perfect cube." << std::endl;
    }

    // データ読み込み
    std::vector<char> data(totalElements);
    if (!file.read(data.data(), fileSize))
    {
        std::cerr << "[ERROR] Failed to read file data." << std::endl;
    }

    // 3次元ベクトルに変換
    this->data = std::vector<std::vector<std::vector<char>>>(this->size, std::vector<std::vector<char>>(this->size, std::vector<char>(this->size)));
    for (size_t i = 0; i < this->size; ++i)
    {
        for (size_t j = 0; j < this->size; ++j)
        {
            for (size_t k = 0; k < this->size; ++k)
            {
                this->data[i][j][k] = data[i * this->size * this->size + j * this->size + k];
            }
        }
    }
}