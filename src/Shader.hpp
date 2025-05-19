#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>

class Shader
{
private:
    GLuint programID = 0;

    // シェーダーソース読み込み
    std::string LoadShaderSource(const std::string &filepath)
    {
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            std::cerr << "Failed to open shader file: " << filepath << std::endl;
            return "";
        }

        std::stringstream shaderStream;
        shaderStream << file.rdbuf();
        return shaderStream.str();
    }

    // シェーダーコンパイル
    GLuint CompileShader(const std::string &source, GLenum type)
    {
        GLuint shader = glCreateShader(type);
        const char *src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            std::cerr << "Shader Compilation Error: " << infoLog << std::endl;
        }

        return shader;
    }

public:
    // コンストラクタ
    Shader(const std::string &vertexPath, const std::string &fragmentPath)
    {
        std::string vertexSource = LoadShaderSource(vertexPath);
        std::string fragmentSource = LoadShaderSource(fragmentPath);

        GLuint vertexShader = CompileShader(vertexSource, GL_VERTEX_SHADER);
        GLuint fragmentShader = CompileShader(fragmentSource, GL_FRAGMENT_SHADER);

        programID = glCreateProgram();
        glAttachShader(programID, vertexShader);
        glAttachShader(programID, fragmentShader);
        glLinkProgram(programID);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    ~Shader()
    {
        glDeleteProgram(programID);
    }

    // シェーダープログラムの使用
    void Use() const { glUseProgram(programID); }

    // プログラムIDの取得
    GLuint GetProgramID() const { return programID; }
};
