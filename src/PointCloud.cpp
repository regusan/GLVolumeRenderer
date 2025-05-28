#include <iostream>
#include <numeric>
#include <algorithm>
#include <vector>
#include <unordered_map>

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

/// @brief 正しい深度ソート（ビュー座標系のZ値基準）を行う関数 (奥から手前へ)
/// @param vertices 全ての頂点の3D座標リスト
/// @param indices_to_sort ソート対象の頂点インデックスリスト (通常は 0 から N-1)
/// @param MV モデルビュー行列
/// @return 深度ソートされた頂点インデックスリスト (奥から手前)
std::vector<GLuint> CorrectDepthSort(
    const std::vector<Vertex> &vertices,
    const std::vector<GLuint> &indices_to_sort,
    const glm::mat4 &MV)
{
    size_t n = indices_to_sort.size();
    if (n == 0)
    {
        return {}; // 空なら空を返す
    }

    // 1. 深度とインデックスのペアを格納するベクターを作成
    std::vector<std::pair<float, GLuint>> depth_index_pairs;
    depth_index_pairs.reserve(n);

    // 2. 各頂点の深度を計算し、ペアを作成
    for (GLuint index : indices_to_sort)
    {
        // 頂点インデックスが頂点リストの範囲内かチェック (念のため)
        if (index >= vertices.size())
        {
            // エラー処理 (ここではスキップまたは例外スロー)
            std::cerr << "Warning: Index " << index << " is out of bounds." << std::endl;
            continue;
        }

        // 3D座標を取得
        const glm::vec3 &vertex_pos = vertices[index].position;

        // 4Dベクトルに変換 (w=1.0) してMV行列を適用
        glm::vec4 vertex_view = MV * glm::vec4(vertex_pos, 1.0f);

        // Z値（深度）とインデックスをペアにして追加
        // vertex_view.z がビュー座標系の深度
        depth_index_pairs.push_back({vertex_view.z, index});
    }

    // 3. 深度 (ペアの最初の要素) に基づいてソート
    // std::pair はデフォルトで最初の要素を基準に昇順ソートする。
    // Z値が小さい（より負に近い、つまり遠い）順になるため、
    // これで「奥から手前へ」のソートになる。
    std::sort(depth_index_pairs.begin(), depth_index_pairs.end());

    // 4. ソートされたインデックスを抽出
    std::vector<GLuint> sorted_indices;
    sorted_indices.reserve(depth_index_pairs.size());
    for (const auto &pair : depth_index_pairs)
    {
        sorted_indices.push_back(pair.second); // ペアの2番目の要素（インデックス）を追加
    }

    return sorted_indices;
}

/// @brief 2つの GLuint 配列間の平均絶対差 (MAD) を計算する関数
/// @param A 基準となるソート済み配列
/// @param B 比較対象のソート済み配列
/// @return 平均絶対差 (MAD)
double calculateMeanAbsoluteDifference(const std::vector<GLuint> &A, const std::vector<GLuint> &B)
{
    // 1. サイズチェック
    if (A.size() != B.size())
    {
        throw std::runtime_error("Error: Input vectors must have the same size.");
    }

    size_t n = A.size();
    if (n == 0)
    {
        return 0.0; // 空の配列の場合、差は0
    }

    // 2. 配列 A のインデックスマップを作成
    std::unordered_map<GLuint, size_t> mapA;
    mapA.reserve(n);
    for (size_t i = 0; i < n; ++i)
    {
        mapA[A[i]] = i;
    }

    // 3. 差分の合計計算
    long long sum_abs_diff = 0; // 合計は大きくなる可能性があるので long long を使用
    for (size_t i = 0; i < n; ++i)
    {
        GLuint element_B = B[i];

        // B の要素が A に存在するかチェック
        auto it = mapA.find(element_B);
        if (it == mapA.end())
        {
            throw std::runtime_error("Error: Vector B contains an element not present in A.");
        }

        size_t index_A = it->second;
        size_t index_B = i;

        // 差の絶対値を加算 (size_t同士の引き算は危険なので、符号付き整数にキャスト)
        sum_abs_diff += std::abs(static_cast<long long>(index_A) - static_cast<long long>(index_B));
    }

    // 4. 平均の計算 (doubleで計算するためにキャスト)
    return static_cast<double>(sum_abs_diff) / n;
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
    size_t idxSize = X.size();
    // 視点順に並び替えられたインデックス
    std::vector<GLuint> reordered;
    reordered.reserve(idxSize);

    // 視点正面ベクトルを取得
    const glm::vec3 viewDir = glm::normalize(glm::vec3(glm::inverse(MV)[2])) + glm::vec3(0.00001f); // 発散防止
    // cout << viewDir.x << ", " << viewDir.y << ", " << viewDir.z << endl;
    //  インクリメントされていく各軸配列の先頭オフセット
    sizevec3 head = sizevec3(0);
    // 蓄積された軸方向の重み(符号なし)
    glm::vec3 err = glm::vec3(0);
    // そのインデックスがすでに登録されたかどうか
    vector<bool> used(idxSize, false);

    while (reordered.size() < idxSize)
    {
        // 軸方向の重みを符号なしで加算
        err += abs(viewDir);

        if (head.x < idxSize && err.x > 1.0f)
        {
            // 処理対象とするインデックス番号。viewDirが負なら末尾から取得していく
            GLuint idx = (viewDir.x >= 0) ? X[head.x] : X[idxSize - head.x - 1];
            // 未使用のインデックスまで進める
            while (head.x < idxSize && used[idx])
            {
                idx = (viewDir.x > 0) ? X[head.x] : X[idxSize - head.x - 1];
                head.x++;
            }
            reordered.push_back(idx); // 未使用のインデックスを追加
            used[idx] = true;         // インデックスを使用済みにする
            err.x -= 1.0f;
        }
        if (head.y < idxSize && err.y >= 1.0f)
        {
            GLuint idx = (viewDir.y > 0) ? Y[head.y] : Y[idxSize - head.y - 1];
            while (head.y < idxSize && used[idx])
            {
                idx = (viewDir.y > 0) ? Y[head.y] : Y[idxSize - head.y - 1];
                head.y++;
            }
            reordered.push_back(idx);
            used[idx] = true;
            err.y -= 1.0f;
        }
        if (head.z < idxSize && err.z >= 1.0f)
        {
            GLuint idx = (viewDir.z > 0) ? Z[head.z] : Z[idxSize - head.z - 1];
            while (head.z < idxSize && used[idx])
            {
                idx = (viewDir.z > 0) ? Z[head.z] : Z[idxSize - head.z - 1];
                head.z++;
            }
            reordered.push_back(idx);
            used[idx] = true;
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
    vertices.reserve(static_cast<size_t>(data.size() * data.size() * data.size() * 0.1f)); // 10%でとりあえずアロケート
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
                    vertices.push_back(Vertex{
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

    vector<GLuint> indices(reordered.size());
    std::iota(indices.begin(), indices.end(), 0);
    auto correct = CorrectDepthSort(this->vertices, indices, view);

    auto wrongs = calculateMeanAbsoluteDifference(reordered, correct);
    cout << u8"近似ソートの平均インデックス誤差:" << wrongs << "/" << correct.size() << u8",誤り率" << static_cast<double>(wrongs) * 100 / correct.size() << "%" << endl;

    // 描画設定
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    // glEnable(GL_POINT_SPRITE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glPointSize(1.0f);
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

    glBindVertexArray(vao);
    glDrawElements(GL_POINTS, indicesX.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}