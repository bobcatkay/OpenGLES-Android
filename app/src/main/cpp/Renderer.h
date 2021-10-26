//
// Created by xulinkai on 2021/10/24.
//

#ifndef OPENGLES_ANDROID_RENDERER_H
#define OPENGLES_ANDROID_RENDERER_H

#include <list>
#include <android/native_window_jni.h>
#include "Shader.h"
#include "Util.h"

class Renderer {
public:
    Renderer(ANativeWindow* window, int surfaceWidth, int surfaceHeight);
    ~Renderer();

    void onDrawFrame(AHardwareBuffer* buffer, int width, int height);
    void Release();

private:
    const EGLint IMAGE_KHR_ATTR[3] = {EGL_IMAGE_PRESERVED_KHR, EGL_TRUE, EGL_NONE};

    Shader* pShader;
    ANativeWindow* pWindow;
    EGLSurface mEglSurface;
    EGLDisplay mDisplay;
    EGLContext mContext;
    int mWindowWidth = 0;
    int mWindowHeight = 0;
    AHardwareBuffer* pLastBuffer = nullptr;
    GLuint mVAO;
    GLuint mVertexCount = 6;
    GLuint mTexId = 0;
    GLuint mLastBufferWidth = 0;
    GLuint mLastBufferHeight = 0;
    glm::mat4 mProjectionMatrix = glm::mat4(1.0f);
    glm::mat4 mTransformMatrix = glm::mat4(1.0f);

    void BindHardwareBuffer(GLuint texId, AHardwareBuffer* buffer);
    void UpdateTexture(AHardwareBuffer* buffer, int width, int height);
    void Init();
    void InitContext();
    void InitVertex();
};


#endif //OPENGLES_ANDROID_RENDERER_H
