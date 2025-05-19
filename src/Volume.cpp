#include "Volume.hpp"
#include <queue>

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
    this->data = VolumeData(this->size, std::vector<std::vector<Cell>>(this->size, std::vector<Cell>(this->size)));
    for (size_t i = 0; i < this->size; ++i)
    {
        for (size_t j = 0; j < this->size; ++j)
        {
            for (size_t k = 0; k < this->size; ++k)
            {
                this->data[i][j][k].intencity = data[i * this->size * this->size + j * this->size + k];
            }
        }
    }

    // Volume::Clustering(this->data);
}

// BFSによる領域探索（スタックオーバーフロー防止）
void Search(Volume::VolumeData &v, size_t x, size_t y, size_t z, unsigned int id)
{
    const size_t size = v.size();
    std::queue<std::tuple<size_t, size_t, size_t>> queue;
    queue.emplace(x, y, z);

    while (!queue.empty())
    {
        auto [cx, cy, cz] = queue.front();
        queue.pop();

        // 範囲外チェック
        if (cx >= size || cy >= size || cz >= size)
            continue;

        Volume::Cell &cell = v[cx][cy][cz];

        if (cell.intencity == 0 || cell.id != 0)
            continue;

        // ID割り当て
        cell.id = id;

        // 6方向に探索
        if (cx + 1 < size)
            queue.emplace(cx + 1, cy, cz);
        if (cx > 0)
            queue.emplace(cx - 1, cy, cz);
        if (cy + 1 < size)
            queue.emplace(cx, cy + 1, cz);
        if (cy > 0)
            queue.emplace(cx, cy - 1, cz);
        if (cz + 1 < size)
            queue.emplace(cx, cy, cz + 1);
        if (cz > 0)
            queue.emplace(cx, cy, cz - 1);
    }
}

void Volume::Clustering(VolumeData &v)
{
    size_t size = v.size();
    unsigned long long idIndex = 1;
    for (size_t i = 0; i < size; ++i)
    {
        for (size_t j = 0; j < size; ++j)
        {
            for (size_t k = 0; k < size; ++k)
            {
                if (v[i][j][k].intencity != 0)
                {
                    // 値がありかつ未割り当てなら割り当開始
                    if (v[i][j][k].intencity > 0 && v[i][j][k].id == 0)
                    {
                        Search(v, i, j, k, idIndex);
                        idIndex++;
                    }
                }
            }
        }
    }
}
std::ostream &operator<<(std::ostream &os, Volume &v)
{
    size_t size = v.size;
    size_t step = size / 15;
    os << size << "x" << size << "x" << size << std::endl;
    for (int i = 0; i < size; i += step)
    {
        for (int j = 0; j < size; j += step)
        {
            for (int k = 0; k < size; k += step)
            {
                os << "(" << static_cast<int>(v.data[i][j][k].intencity) << "," << v.data[i][j][k].id << ") ";
            }
            os << std::endl;
        }
        os << std::endl;
    }
    return os;
}