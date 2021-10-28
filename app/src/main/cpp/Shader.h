//
// Created by xulinkai on 2021/10/24.
//

#ifndef OPENGLES_ANDROID_SHADER_H
#define OPENGLES_ANDROID_SHADER_H

#include <unordered_map>
#include <string>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/ext.hpp"
#include "Util.h"

class Shader {
public:
    Shader() {};
    Shader(const char *vertexCode, const char *fragCode);
    ~Shader();

    void UseProgram();
    void SetMat4(const std::string& name, const glm::mat4& mat);
    void SetMat3(const std::string& name, const float* value);
    void SetInt(const std::string& name, int value);
    GLuint GetProgram() {return mProgram;}

private:
    GLuint mProgram = 0;
    std::unordered_map <std::string, GLint> mUniformLocations;

    void InitShaderProgram(const char* vertexCode, const char* fragCode);
    GLuint LoadShader(const char* code, GLenum type);
    GLint GetUniformLocation(const std::string& name);
};


#endif //OPENGLES_ANDROID_SHADER_H
