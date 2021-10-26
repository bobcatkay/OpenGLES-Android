//
// Created by xulinkai on 2021/10/24.
//

#include "Shader.h"

Shader::~Shader() {
    if (mProgram != 0) {
        glDeleteProgram(mProgram);
        mProgram = 0;
    }
}

Shader::Shader(const char *vertexCode, const char *fragCode) {
    InitShaderProgram(vertexCode, fragCode);
}

void Shader::SetMat4(const std::string& name, const glm::mat4& mat) {
    GLint location = GetUniformLocation(name);

    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
    }
}

void Shader::SetInt(const std::string& name, int value) {
    GLint location = GetUniformLocation(name);

    if (location > 0) {
        glUniform1i(location, value);
    }
}

GLint Shader::GetUniformLocation(const std::string& name) {
    if (mUniformLocations.find(name) != mUniformLocations.end()) {
        return mUniformLocations[name];
    }

    GLint location = glGetUniformLocation(mProgram, name.c_str());
    mUniformLocations[name] = location;

    return location;
}

GLuint Shader::LoadShader(const char* code, GLenum type) {
    GLuint shader = glCreateShader(type);

    if (0 == shader) {
        LOGE("glCreateShader %d failed!", type);
        return 0;
    }

    // Load the shader source
    glShaderSource(shader, 1, &code, nullptr);

    // Compile the shader
    glCompileShader(shader);

    // Check the compile status
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (!status) {
        GLchar eLog[1024] = { 0 };
        glGetShaderInfoLog(shader, sizeof(eLog), NULL, eLog);
        LOGE("LoadShader, Error compiling the %d shader: '%s'", type, eLog);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

void Shader::InitShaderProgram(const char *vertexCode, const char *fragCode) {
    mProgram = glCreateProgram();

    if (0 == mProgram) {
        LOGE("InitShaderProgram, create mShaderProgram failed!");
        return;
    }

    GLint vertexShader = LoadShader(vertexCode, GL_VERTEX_SHADER);
    GLint fragShader = LoadShader(fragCode, GL_FRAGMENT_SHADER);

    glAttachShader(mProgram, vertexShader);
    glAttachShader(mProgram, fragShader);

    glLinkProgram(mProgram);

    GLint linked;
    glGetProgramiv(mProgram, GL_LINK_STATUS, &linked);

    if (!linked) {
        GLchar eLog[1024] = { 0 };
        glGetProgramInfoLog(mProgram, sizeof(eLog), NULL, eLog);
        LOGE("InitShaderProgram, Error linking mShaderProgram: '%s'", eLog);
        glDeleteProgram(mProgram);
    }
}

void Shader::UseProgram() {
    glUseProgram(mProgram);
}
