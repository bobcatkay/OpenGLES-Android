//
// Created by xulinkai on 2021/10/24.
//

#ifndef OPENGLES_ANDROID_RENDERER_H
#define OPENGLES_ANDROID_RENDERER_H

#include <list>
#include <EGL/egl.h>
#include <android/native_window_jni.h>
#include "Shader.h"
#include "Util.h"

class Renderer {
public:
    Renderer(ANativeWindow* window, int surfaceWidth, int surfaceHeight);
    ~Renderer();

    void AddBuffer(AHardwareBuffer* buffer);
    void ShutDown();

private:
    Shader* pShader;
    ANativeWindow* pWindow;
    EGLSurface mEglSurface;
    EGLDisplay mDisplay;
    EGLContext mContext;
    int mWindowWidth = 0;
    int mWindowHeight = 0;
    bool mbShutDown = false;
    std::list<AHardwareBuffer*> mBufferList;
    AHardwareBuffer* pBuffer;
    GLuint mVAO;
    GLuint mVertexCount = 6;
    GLuint mTexId = 0;

    void InitContext();
    void InitVertex();
    void Run();
    void onDraw();
};


#endif //OPENGLES_ANDROID_RENDERER_H
