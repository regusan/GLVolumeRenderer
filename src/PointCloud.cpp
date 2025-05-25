#include <iostream>
#include <numeric>
#include <algorithm>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "PointCloud.hpp"
#include "Volume.hpp"

using namespace std;

/// @brief 軸に沿ったインデクスのソート済み配列を生成する
/// @tparam Compare ラムダ式のキャプチャ式を使うためのテンプレート
/// @param vertices 頂点
/// @param indices 代入される頂点インデクス
/// @param compare 比較ラムダ式
template <typename Compare>
void CreateAxisAlignedSortedIndices(std::vector<Vertex> &vertices, std::vector<GLuint> &indices, Compare &&compare)
{
    indices.resize(vertices.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), std::forward<Compare>(compare));
}

// TODO::並列実行できるように
/// @brief 各軸にソートされた頂点インデックスと視点ベクトルを基に深度ソートを近似する関数
/// @param X X軸方向にソートされた頂点インデックス
/// @param Y Y軸
/// @param Z Z軸
/// @param view ビュー行列
/// @return ソートされた頂点インデックス
std::vector<GLuint> PointCloud::ReorderIndices(const std::vector<GLuint> &X, const std::vector<GLuint> &Y, const std::vector<GLuint> &Z, const glm::mat4 &MV)
{
    using sizevec3 = glm::vec<3, size_t, glm::defaultp>;

    // 視点順に並び替えられたインデックス
    std::vector<GLuint> reordered;
    reordered.reserve(X.size());

    // 視点正面ベクトルを取得
    const glm::vec3 viewDir = glm::normalize(glm::vec3(glm::inverse(MV)[2])) + glm::vec3(0.00001f); // 発散防止
    // cout << viewDir.x << ", " << viewDir.y << ", " << viewDir.z << endl;
    //  インクリメントされていく各軸配列の先頭オフセット
    sizevec3 head = sizevec3(0);
    // 蓄積された軸方向の重み(符号なし)
    glm::vec3 err = glm::vec3(0);
    // そのインデックスがすでに登録されたかどうか
    vector<bool> used(X.size(), false);

    while (reordered.size() <= X.size())
    {
        // 軸方向の重みを符号なしで加算
        err += abs(viewDir);

        if (head.x < X.size() && err.x > 1.0f)
        {
            // 処理対象とするインデックス番号。viewDirが負なら末尾から取得していく
            GLuint idx = (viewDir.x > 0) ? X[head.x] : X[X.size() - head.x - 1];
            // 未使用のインデックスまで進める
            while (head.x < X.size() && used[idx])
            {
                head.x++;
                idx = (viewDir.x > 0) ? X[head.x] : X[X.size() - head.x - 1];
            }
            reordered.push_back(idx); // 未使用のインデックスを追加
            used[idx] = true;         // インデックスを使用済みにする
            head.x++;                 // 次のインデックスへ
            err.x -= 1.0f;
        }
        if (head.y < Y.size() && err.y > 1.0f)
        {
            GLuint idx = (viewDir.y > 0) ? Y[head.y] : Y[Y.size() - head.y - 1];
            while (head.y < Y.size() && used[idx])
            {
                head.y++;
                idx = (viewDir.y > 0) ? Y[head.y] : Y[Y.size() - head.y - 1];
            }
            reordered.push_back(idx);
            used[idx] = true;
            head.y++;
            err.y -= 1.0f;
        }
        if (head.z < Z.size() && err.z > 1.0f)
        {
            GLuint idx = (viewDir.z > 0) ? Z[head.z] : Z[Z.size() - head.z - 1];
            while (head.z < Z.size() && used[idx])
            {
                head.z++;
                idx = (viewDir.z > 0) ? Z[head.z] : Z[Z.size() - head.z - 1];
            }
            reordered.push_back(idx);
            used[idx] = true;
            head.z++;
            err.z -= 1.0f;
        }
    }
    return reordered;
}

PointCloud::PointCloud(const Volume &volume)
{
    // ボリュームデータを点群データに変換
    this->vertices = PointCloud::VolumeToVertices(volume.data);
    // 各軸方向にインデックスをソート
    CreateAxisAlignedSortedIndices(vertices, indicesX, [&](GLuint a, GLuint b)
                                   { return vertices[a].position.x < vertices[b].position.x; });
    CreateAxisAlignedSortedIndices(vertices, indicesY, [&](GLuint a, GLuint b)
                                   { return vertices[a].position.y < vertices[b].position.y; });
    CreateAxisAlignedSortedIndices(vertices, indicesZ, [&](GLuint a, GLuint b)
                                   { return vertices[a].position.z < vertices[b].position.z; });
}

PointCloud::PointCloud(/* args */)
{
}

PointCloud::~PointCloud()
{
    if (vao)
        glDeleteVertexArrays(1, &vao);
    if (vbo)
        glDeleteBuffers(1, &vbo);
    if (ibo)
        glDeleteBuffers(1, &ibo);
}

vector<Vertex> PointCloud::VolumeToVertices(const Volume::VolumeData &data)
{
    vector<Vertex> vertices;
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
    glGenBuffers(1, &this->ibo);

    glBindVertexArray(this->vao);

    // 頂点データのアップロード
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
    glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), this->vertices.data(), GL_STATIC_DRAW);

    // インデックスデータのアップロード
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indicesX.size() * sizeof(GLuint), this->indicesX.data(), GL_STATIC_DRAW);

    // 頂点属性の設定
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, position)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, alpha)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void PointCloud::Draw(const glm::mat4 &view)
{

    // 毎フレーム IBO を更新（描画順の変更）
    const vector<GLuint> reordered = PointCloud::ReorderIndices(indicesX, indicesY, indicesZ, view);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, reordered.size() * sizeof(GLuint), reordered.data(), GL_DYNAMIC_DRAW);

    // 描画設定
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(1.0f);

    glBindVertexArray(vao);
    glDrawElements(GL_POINTS, reordered.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}