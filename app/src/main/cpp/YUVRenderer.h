//
// Created by xulinkai on 2021/10/27.
//
#ifndef OPENGLES_ANDROID_YUVRENDERER_H
#define OPENGLES_ANDROID_YUVRENDERER_H

#include <android/native_window_jni.h>
#include "Shader.h"
#include "Texture.h"

class YUVRenderer {
public:
    YUVRenderer(ANativeWindow* window, int surfaceWidth, int surfaceHeight);
    ~YUVRenderer() {};

    void OnDrawFrame();
    void Release();
    void ShutDown();

private:
    const GLuint VERTEX_COUNT = 6;

    Shader* pShader;
    ANativeWindow* pWindow;
    EGLSurface mEglSurface;
    EGLDisplay mDisplay;
    EGLContext mContext;
    int mWindowWidth = 0;
    int mWindowHeight = 0;
    GLuint mVAO;
    glm::mat4 mProjectionMatrix = glm::mat4(1.0f);
    glm::mat4 mTransformMatrix = glm::mat4(1.0f);
    Texture* mTexture[3] = {0};
    bool mbShutDown = false;

    void Init();
    void FreeResources();
    void UpdateTexture();
    void InitYUVTexture();
};


#endif //OPENGLES_ANDROID_YUVRENDERER_H
