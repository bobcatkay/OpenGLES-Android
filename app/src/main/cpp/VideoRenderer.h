//
// Created by xulinkai on 2021/10/27.
//
#ifndef OPENGLES_ANDROID_VIDEORENDERER_H
#define OPENGLES_ANDROID_VIDEORENDERER_H

#include <android/native_window_jni.h>
#include "Shader.h"
#include "Util.h"
#include "VideoDecoder.h"

class VideoRenderer {
public:
    VideoRenderer(ANativeWindow* window, int surfaceWidth, int surfaceHeight, int videoFileFD);
    ~VideoRenderer() {};

    void OnDrawFrame();
    void Release();
    void VideoDecodeCallback(AVFrame* frame);

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
    Texture mTexture[3];
    VideoDecoder* pVideoDecoder;
    AVFrame* mFrame;
    int mVideoWidth;
    int mVideoHeight;
    bool bVideoSizeInit = false;

    void Init();
    void StartDecoder(int videoFileFD);
    void FreeResources();
    void UpdateTexture();
    void InitYUVTexture();
};


#endif //OPENGLES_ANDROID_VIDEORENDERER_H
