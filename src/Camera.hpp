#pragma once

#include <GLFW/glfw3.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/// XXX:複数のインスタンスが出現すると機能しなくなる！！！シングルトンクラスでサービス化しろ！
float MOUSE_YOFFSET = 0;

///
void ScrollCallback(GLFWwindow *, double, double _yoffset) { MOUSE_YOFFSET = _yoffset; }

/// @brief カメラのクラス。実質シングルトン状態
class Camera
{
private:
    GLFWwindow *window;
    double armLength = 1.0f;
    glm::dvec2 currentRotation = {0, 0};
    glm::dvec3 currentPosition = {0, 0, 0};
    const glm::dvec2 resetPos = {-1000, -1000};
    glm::dvec2 lastCursorRotPos = Camera::resetPos;
    glm::dvec2 lastCursorPosPos = Camera::resetPos;
    float cameraSpeed = 0.01;

public:
    glm::mat4 view;
    Camera(GLFWwindow *window) : window(window)
    {
        this->window = window;
        glfwSetWindowUserPointer(window, this);
        glfwSetScrollCallback(window, ScrollCallback);
    }
    virtual void Update()
    { // マウスの左ボタンが押されているか確認
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            glm::dvec2 currentCursorPos;
            glfwGetCursorPos(window, &currentCursorPos.x, &currentCursorPos.y);

            // 二回目以降で押されていたらカメラ回転
            if (this->lastCursorRotPos != Camera::resetPos)
            {
                currentRotation += currentCursorPos - this->lastCursorRotPos;
                this->lastCursorRotPos = currentCursorPos;
            }
            // 初めて押された時は移動しない
            else
            {
                this->lastCursorRotPos = currentCursorPos;
            }
        }
        else
        {
            lastCursorRotPos = Camera::resetPos;
        }

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
        {
            glm::dvec2 currentCursorPos;
            glfwGetCursorPos(window, &currentCursorPos.x, &currentCursorPos.y);

            // 二回目以降で押されていたらカメラ移動
            if (this->lastCursorPosPos != Camera::resetPos)
            {
                glm::dvec2 delta = currentCursorPos - this->lastCursorPosPos;
                this->currentPosition += glm::vec3(glm::inverse(view)[0]) * static_cast<float>(delta.x) * cameraSpeed +
                                         glm::vec3(glm::inverse(view)[1]) * static_cast<float>(delta.y) * cameraSpeed;
                this->lastCursorPosPos = currentCursorPos;
            }
            // 初めて押された時は移動しない
            else
            {
                this->lastCursorPosPos = currentCursorPos;
            }
        }
        else
        {
            lastCursorPosPos = Camera::resetPos;
        }
        this->armLength *= 1.0 - MOUSE_YOFFSET * 0.1;
        view = glm::mat4(1);
        view = glm::translate(view, glm::vec3(currentPosition.x, currentPosition.y, currentPosition.z - armLength));
        view = glm::rotate(view, glm::radians<float>(currentRotation.y * 1), glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, glm::radians<float>(currentRotation.x * 1), glm::vec3(0.0f, 1.0f, 0.0f));
        // view = glm::inverse(view);//本来なら逆行列を取るが、中心点周りを周遊するカメラを作りたいため、あえてInvertしない

        MOUSE_YOFFSET = 0;
    }

    glm::vec3 GetPos() const
    {
        return glm::vec3(this->currentPosition.x, currentPosition.y, currentPosition.z - armLength);
    }

    ~Camera();
};

Camera::~Camera()
{
}
