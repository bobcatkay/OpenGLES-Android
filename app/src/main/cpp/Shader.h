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

//#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "Shader" ,__VA_ARGS__)
//#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "Shader" ,__VA_ARGS__)

class Shader {
public:
    Shader() {};
    Shader(const char *vertexCode, const char *fragCode);
    ~Shader();

    void UseProgram();
    void SetMat4(std::string name, const glm::mat4& mat);
    void SetInt(std::string name, int value);

private:
    GLuint mProgram = 0;
    std::unordered_map <std::string, GLint> mUniformLocations;

    void InitShaderProgram(const char* vertexCode, const char* fragCode);
    GLuint LoadShader(const char* code, GLenum type);
    GLint GetUniformLocation(std::string name);
};


#endif //OPENGLES_ANDROID_SHADER_H
