//
// Created by xulinkai on 2021/10/27.
//
#ifndef OPENGLES_ANDROID_VIDEORENDERER_H
#define OPENGLES_ANDROID_VIDEORENDERER_H

#include <android/native_window_jni.h>
#include "Shader.h"
#include "Texture.h"
#include "VideoDecoder.h"

class VideoRenderer {
public:
    VideoRenderer(ANativeWindow* window, int surfaceWidth, int surfaceHeight);
    ~VideoRenderer() {};

    void OnDrawFrame();
    void Shutdown();
    void VideoDecodeCallback(AVFrame* frame);
    void SetVideoSize(int width, int height);
    void OnSurfaceChanged(int surfaceWidth, int surfaceHeight);

private:
    const GLuint VERTEX_COUNT = 6;

    Shader* pShader;
    ANativeWindow* pWindow;
    EGLSurface mEglSurface;
    EGLDisplay mDisplay;
    EGLContext mContext;
    int mWindowWidth {0};
    int mWindowHeight {0};
    GLuint mVAO;
    glm::mat4 mProjectionMatrix = glm::mat4(1.0f);
    glm::mat4 mTransformMatrix = glm::mat4(1.0f);
    Texture* mTexture[3] = {0};
    AVFrame* mFrame = nullptr;
    int mVideoWidth {0};
    int mVideoHeight {0};
    bool mbDataReceived = false;
    bool mbShutdown = false;

    void Init();
    void UpdateTexture();
    void InitYUVTexture();
    void InitMatrix();
    void Release();
};


#endif //OPENGLES_ANDROID_VIDEORENDERER_H
