//
// Created by xulinkai on 2021/10/24.
//

#ifndef OPENGLES_ANDROID_CAMERARENDERER_H
#define OPENGLES_ANDROID_CAMERARENDERER_H

#include <android/native_window_jni.h>
#include "Shader.h"
#include "Util.h"

class CameraRenderer {
public:
    CameraRenderer(ANativeWindow* window, int surfaceWidth, int surfaceHeight);
    ~CameraRenderer() {};

    void OnDrawFrame(AHardwareBuffer* buffer, int width, int height);
    void Release();

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
    GLuint mTexId = 0;
    GLuint mLastBufferWidth = 0;
    GLuint mLastBufferHeight = 0;
    glm::mat4 mProjectionMatrix = glm::mat4(1.0f);
    glm::mat4 mTransformMatrix = glm::mat4(1.0f);

    void UpdateTexture(AHardwareBuffer* buffer, int width, int height);
    void Init();
};


#endif //OPENGLES_ANDROID_CAMERARENDERER_H
