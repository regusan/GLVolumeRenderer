#include "Volume.hpp"
#include <queue>
#include <tuple>

using namespace std;

Volume::Volume(ifstream &file)
{
    file.seekg(0, ios::end);            // 末尾まで移動
    streamsize fileSize = file.tellg(); // ファイルサイズ取得
    file.seekg(0, ios::beg);            // 先頭まで戻す

    size_t totalElements = static_cast<int>(fileSize);
    this->size = cbrt(totalElements); // Nを推測

    if (this->size * this->size * this->size != totalElements)
    {
        cerr << "[ERROR] File size is not a perfect cube." << endl;
    }

    // データ読み込み
    vector<char> data(totalElements);
    if (!file.read(data.data(), fileSize))
    {
        cerr << "[ERROR] Failed to read file data." << endl;
    }

    // 3次元ベクトルに変換
    this->data = VolumeData(this->size, vector<vector<Cell>>(this->size, vector<Cell>(this->size)));
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

glm::vec3 Volume::CalcNormalAtIndex(size_t _x, size_t _y, size_t _z)
{
    glm::vec3 grad = {this->data[_x - 1][_y][_z].intencity - this->data[_x][_y][_z].intencity,  // X軸方向の強度
                      this->data[_x][_y - 1][_z].intencity - this->data[_x][_y][_z].intencity,  // Y軸
                      this->data[_x][_y][_z - 1].intencity - this->data[_x][_y][_z].intencity}; // Z軸
    return glm::normalize(grad);
}
void Volume::CalcNormal()
{
    for (size_t i = 0; i < this->size; ++i)
    {
        for (size_t j = 0; j < this->size; ++j)
        {
            for (size_t k = 0; k < this->size; ++k)
            {
                // this->data[i][j][k].normal = CalcNormalAtIndex(i, j, k);
            }
        }
    }
}

// BFSによる領域探索（スタックオーバーフロー防止）
void Search(Volume::VolumeData &v, size_t x, size_t y, size_t z, unsigned int id)
{
    const size_t size = v.size();
    queue<tuple<size_t, size_t, size_t>> queue;
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
ostream &operator<<(ostream &os, Volume &v)
{
    size_t size = v.size;
    size_t step = size / 15;
    os << v.Sammary() << endl;
    for (size_t i = 0; i < size; i += step)
    {
        for (size_t j = 0; j < size; j += step)
        {
            for (size_t k = 0; k < size; k += step)
            {
                os << "(" << static_cast<int>(v.data[i][j][k].intencity) << "," << v.data[i][j][k].id << ") ";
            }
            os << endl;
        }
        os << endl;
    }
    return os;
}

string Volume::Sammary()
{
    return to_string(size) + "x" + to_string(size) + "x" + to_string(size) + "=" + to_string(size * size * size);
}
void Volume::Draw()
{
    // 設定
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, this->volumeTexture);

    // フルスクリーンクワッド描画で全ピクセルのピクセルシェーダー起動
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

void Volume::UploadBuffer()
{
    std::vector<float> volumeData(size * size * size);
    for (size_t i = 0; i < size; ++i)
    {
        for (size_t j = 0; j < size; ++j)
        {
            for (size_t k = 0; k < size; ++k)
            {
                volumeData[i * size * size + j * size + k] = static_cast<float>(data[i][j][k].intencity) / 255.0f;
            }
        }
    }

    glGenTextures(1, &volumeTexture);
    glBindTexture(GL_TEXTURE_3D, volumeTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, size, size, size, 0, GL_RED, GL_FLOAT, volumeData.data());

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    float cubeVertices[] = {
        -0.5f,
        -0.5f,
        -0.5f,
        0.5f,
        -0.5f,
        -0.5f,
        0.5f,
        0.5f,
        -0.5f,
        -0.5f,
        0.5f,
        -0.5f,
        -0.5f,
        -0.5f,
        0.5f,
        0.5f,
        -0.5f,
        0.5f,
        0.5f,
        0.5f,
        0.5f,
        -0.5f,
        0.5f,
        0.5f,
    };

    // インデックスでキューブ6面構成（12三角形）
    unsigned int cubeIndices[] = {
        2, 1, 0,
        0, 3, 2,
        6, 5, 1,
        1, 2, 6,
        7, 4, 5,
        5, 6, 7,
        3, 0, 4,
        4, 7, 3,
        6, 2, 3,
        3, 7, 6,
        1, 5, 4,
        4, 0, 1};
    GLuint cubeVBO, cubeEBO;
    glGenVertexArrays(1, &this->cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glGenBuffers(1, &cubeEBO);

    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
}
