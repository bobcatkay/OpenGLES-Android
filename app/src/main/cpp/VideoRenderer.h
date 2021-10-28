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
    VideoRenderer(ANativeWindow* window, int surfaceWidth, int surfaceHeight, int videoFileFD);
    ~VideoRenderer() {};

    void OnDrawFrame();
    void Release();
    void VideoDecodeCallback(AVFrame* frame);

private:
    const GLuint VERTEX_COUNT = 6;
    const GLfloat MATRIX_BT709[9] = {
            1.164,  1.164,  1.164,
            0.0,   -0.213,  2.112,
            1.793, -0.533,  0.0,
    };

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
    VideoDecoder* pVideoDecoder;
    AVFrame* mFrame;
    int mVideoWidth {0};
    int mVideoHeight {0};
    bool bVideoSizeInit = false;

    void Init();
    void StartDecoder(int videoFileFD);
    void FreeResources();
    void UpdateTexture();
    void InitYUVTexture();
};


#endif //OPENGLES_ANDROID_VIDEORENDERER_H
