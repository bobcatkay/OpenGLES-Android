//
// Created by xulinkai on 2021/10/27.
//

#include "VideoRenderer.h"

static std::mutex sMutex;

VideoRenderer::VideoRenderer(ANativeWindow *window, int surfaceWidth, int surfaceHeight, int videoFileFD) {
    pWindow = window;
    mWindowWidth = surfaceWidth;
    mWindowHeight = surfaceHeight;

    Init();
    StartDecoder(videoFileFD);
}

void VideoRenderer::Init() {
    float aspectRatio = mWindowWidth > mWindowHeight ?
                        (float) mWindowWidth / (float) mWindowHeight :
                        (float) mWindowHeight / (float) mWindowWidth;

    if (mWindowWidth > mWindowHeight) {
        mProjectionMatrix = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f);
    } else {
        mProjectionMatrix = glm::ortho(-1.0f, 1.0f, -aspectRatio, aspectRatio, -1.0f, 1.0f);
    }

    InitContext(mDisplay, pWindow, mEglSurface, mContext);

    const std::string vertexCode = ReadFileFromAssets("common.vert");
    const std::string fragCode = ReadFileFromAssets("texture2d_yuv420p.frag");
    pShader = new Shader(vertexCode.c_str(), fragCode.c_str());

    InitQuadVAO(mVAO);
}

void VideoRenderer::OnDrawFrame() {
    glViewport(0, 0, mWindowWidth, mWindowHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f,0.0f,0.0f,1.0f);

    pShader->UseProgram();
    pShader->SetMat4("uProjection", mProjectionMatrix);
    pShader->SetMat4("uTransform", mTransformMatrix);

    UpdateTexture();

    glBindVertexArray(mVAO);
    glDrawElements(GL_TRIANGLES, VERTEX_COUNT, GL_UNSIGNED_INT, 0);

    eglSwapBuffers(mDisplay, mEglSurface);
}

void VideoRenderer::Release() {
    eglDestroySurface(mDisplay, mEglSurface);
    eglDestroyContext(mDisplay, mContext);
    ANativeWindow_release(pWindow);

    FreeResources();

    if (nullptr != pShader) {
        delete(pShader);
    }
}

void VideoRenderer::FreeResources() {
    if (pVideoDecoder) {
        pVideoDecoder->Shutdown();
        delete(pVideoDecoder);
    }

    for (int i = 0; i < 3;i++) {
        if (mTexture[i].id) {
            glDeleteTextures(1, &mTexture[i].id);
        }
    }

    glDeleteBuffers(1, &mVAO);
}

void VideoRenderer::StartDecoder(int videoFileFD) {
    std::lock_guard<std::mutex> lock(sMutex);
    if (!mFrame) {
        mFrame = av_frame_alloc();
    }
    //创建视频解码器
    pVideoDecoder = new VideoDecoder(videoFileFD);
    pVideoDecoder->GetVideoSize(mVideoWidth, mVideoHeight);
    bVideoSizeInit = true;
    auto callback = std::bind(&VideoRenderer::VideoDecodeCallback,this,std::placeholders::_1);
    pVideoDecoder->Start(callback);

    LOGD("StartDecoder, mVideoWidht: %d, mVideoHeight: %d", mVideoWidth, mVideoHeight);
}

void VideoRenderer::VideoDecodeCallback(AVFrame *frame) {
    LOGD("VideoDecodeCallback");
    std::lock_guard<std::mutex> lock(sMutex);
    if (!mFrame) {
        return;
    }

    if (frame->width != mVideoWidth) {
        return;
    }

    av_frame_unref(mFrame);
    av_frame_move_ref(mFrame, frame);
    LOGD("VideoDecodeCallback, Exit.");
}

void VideoRenderer::UpdateTexture() {
    LOGD("UpdateTexture");
    if (!mVideoWidth){
        return;
    }
    if (!mTexture[0].id) {
        InitYUVTexture();
    }

    sMutex.lock();
    LOGD("UpdateTexture, mTexture[0].width: %d, mFrame->width: %d", mTexture[0].width, mFrame->width);
    if (!mTexture[0].id || mTexture[0].width != mFrame->width) {
        sMutex.unlock();
        return;
    }

    LOGD("UpdateTexture, format: %d, AV_PIX_FMT_YUV420P: %d, AV_PIX_FMT_YUVJ420P: %d",
         mFrame->format, AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUVJ420P);

    if ((mFrame->format == AV_PIX_FMT_YUV420P) || (mFrame->format == AV_PIX_FMT_YUVJ420P)) {
        for (int i = 0; i < 3; i++) {
            Texture &tex = mTexture[i];
            glActiveTexture(GL_TEXTURE0 + tex.location);
            glBindTexture(GL_TEXTURE_2D, tex.id);
            uint8_t *pixels = mFrame->data[i];
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex.width, tex.height, tex.format, GL_UNSIGNED_BYTE, pixels);

            LOGD("UpdateTexture, texture{id: %d, location: %d, with: %d, height: %d}", tex.id, tex.location, tex.width, tex.height);
        }
    }

    sMutex.unlock();
}


void VideoRenderer::InitYUVTexture() {
    LOGD("InitYUVTexture");
    std::lock_guard<std::mutex> lock(sMutex);
    int fmt = mFrame->format;
    if (!mFrame || fmt < 0) {
        return;
    }

    glDisable(GL_BLEND);
    mTexture[0] = {0, 0, mVideoWidth, mVideoHeight, GL_LUMINANCE};
    mTexture[1] = {0, 1, mVideoWidth / 2, mVideoHeight / 2, GL_LUMINANCE};
    mTexture[2] = {0, 2, mVideoWidth / 2, mVideoHeight / 2, GL_LUMINANCE};
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GenTexture(mTexture[0]);
    GenTexture(mTexture[1]);
    GenTexture(mTexture[2]);

    pShader->UseProgram();
    pShader->SetInt("yTex", mTexture[0].location);
    pShader->SetInt("uTex", mTexture[1].location);
    pShader->SetInt("vTex", mTexture[2].location);
    LOGD("InitYUVTexture, textres: [%d, %d, %d]", mTexture[0].id, mTexture[1].id, mTexture[2].id);
}
