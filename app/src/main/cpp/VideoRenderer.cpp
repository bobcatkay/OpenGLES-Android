//
// Created by xulinkai on 2021/10/27.
//

#include "VideoRenderer.h"

static std::mutex sFrameMutex;
static std::mutex sMatrixMutex;

VideoRenderer::VideoRenderer(ANativeWindow *window, int surfaceWidth, int surfaceHeight) {
    pWindow = window;
    mWindowWidth = surfaceWidth;
    mWindowHeight = surfaceHeight;
    mFrame = av_frame_alloc();

    std::thread t(&VideoRenderer::Init, this);
    t.detach();
}

void VideoRenderer::Init() {
    InitContext(mDisplay, pWindow, mEglSurface, mContext);

    const std::string vertexCode = ReadFileFromAssets("common.vert");
    const std::string fragCode = ReadFileFromAssets("texture2d_yuv420p.frag");
    pShader = new Shader(vertexCode.c_str(), fragCode.c_str());

    InitQuadVAO(mVAO);

    while (!mbShutdown) {
        OnDrawFrame();
    }

    Release();
}

void VideoRenderer::OnDrawFrame() {
    glViewport(0, 0, mWindowWidth, mWindowHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    pShader->UseProgram();

    sMatrixMutex.lock();
    pShader->SetMat4("uProjection", mProjectionMatrix);
    pShader->SetMat4("uTransform", mTransformMatrix);
    sMatrixMutex.unlock();

    UpdateTexture();

    if (!mbDataReceived) {
        return;
    }

    for (int i=0; i<3; i++) {
        if (mTexture[i]) {
            mTexture[i]->ActiveTexture();
        }
    }

    glBindVertexArray(mVAO);
    glDrawElements(GL_TRIANGLES, VERTEX_COUNT, GL_UNSIGNED_INT, 0);

    eglSwapBuffers(mDisplay, mEglSurface);
}

void VideoRenderer::VideoDecodeCallback(AVFrame *frame) {
    //LOGD("VideoDecodeCallback");
    sFrameMutex.lock();
    if (!mFrame) {
        sFrameMutex.unlock();
        return;
    }

    if (frame->width != mVideoWidth) {
        sFrameMutex.unlock();
        return;
    }

    av_frame_unref(mFrame);
    av_frame_move_ref(mFrame, frame);
    sFrameMutex.unlock();
    //LOGD("VideoDecodeCallback, Exit.");
}

void VideoRenderer::UpdateTexture() {
    //LOGD("VideoRenderer::UpdateTexture, mVideoWidth: %d, mVideoHeight: %d", mVideoWidth, mVideoHeight);

    if (!mVideoWidth){
        return;
    }

    if (!mTexture[0]) {
        InitYUVTexture();
    }

    sFrameMutex.lock();

    if (!mTexture[0] || !mFrame || (mTexture[0]->width != mFrame->width)) {
        sFrameMutex.unlock();
        return;
    }


    if (mFrame->format >= 0) {
        for (int i = 0; i < 3; i++) {
            mTexture[i]->UpdateData(mFrame->data[i]);
            mbDataReceived = true;
        }
    }

    sFrameMutex.unlock();
}


void VideoRenderer::InitYUVTexture() {
    LOGD("InitYUVTexture");
    std::lock_guard<std::mutex> lock(sFrameMutex);
    if (!mFrame || !mVideoWidth) {
        return;
    }

    mTexture[0] = Texture::GenSingleChannelTexture(mVideoWidth, mVideoHeight, nullptr, 1);
    mTexture[1] = Texture::GenSingleChannelTexture(mVideoWidth / 2, mVideoHeight / 2, nullptr, 2);
    mTexture[2] = Texture::GenSingleChannelTexture(mVideoWidth / 2, mVideoHeight / 2, nullptr, 3);

    pShader->UseProgram();
    pShader->SetInt("yTex", mTexture[0]->unit);
    pShader->SetInt("uTex", mTexture[1]->unit);
    pShader->SetInt("vTex", mTexture[2]->unit);
    LOGD("InitYUVTexture, textures: [%s, %s, %s]", mTexture[0]->ToString(), mTexture[1]->ToString(), mTexture[2]->ToString());
}

void VideoRenderer::InitMatrix() {
    mProjectionMatrix = glm::mat4(1.0f);
    float aspectRatio = mWindowWidth > mWindowHeight ?
                        (float) mWindowWidth / (float) mWindowHeight :
                        (float) mWindowHeight / (float) mWindowWidth;

    if (mWindowWidth > mWindowHeight) {
        mProjectionMatrix = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f);
    } else {
        mProjectionMatrix = glm::ortho(-1.0f, 1.0f, -aspectRatio, aspectRatio, -1.0f, 1.0f);
    }

    float scaleY = 1.0f;
    float scaleX = 1.0f;

    float texRatio = (float) mVideoHeight / mVideoWidth;
    float targetHeight = mWindowWidth * texRatio;

    if (targetHeight <= mWindowHeight) {
        scaleX = (mWindowWidth <= mWindowHeight) ? 1.0f : ((float) mWindowWidth / mWindowHeight);
        scaleY = (mWindowWidth <= mWindowHeight) ? (targetHeight / mWindowWidth) : (targetHeight / mWindowHeight);
    } else {
        float targetWidth = (float) mWindowHeight / texRatio;
        scaleX = (mWindowWidth <= mWindowHeight) ? (targetWidth / mWindowWidth) : (targetWidth / mWindowHeight);
        scaleY = (mWindowWidth <= mWindowHeight) ? ((float) mWindowHeight / mWindowWidth) : 1.0f;
    }

    mTransformMatrix = glm::mat4(1.0f);
    mTransformMatrix = glm::scale(mTransformMatrix, glm::vec3(scaleX, scaleY, 1.0f));
}

void VideoRenderer::SetVideoSize(int width, int height) {
    mVideoWidth = width;
    mVideoHeight = height;

    sMatrixMutex.lock();
    InitMatrix();
    sMatrixMutex.unlock();
}

void VideoRenderer::Shutdown() {
    mbShutdown = true;
}

void VideoRenderer::Release() {
    LOGD("VideoRenderer::Release.");

    for (auto& t: mTexture) {
        delete(t);
    }

    glDeleteBuffers(1, &mVAO);

    if (mFrame) {
        av_frame_free(&mFrame);
    }

    if (pShader) {
        delete(pShader);
    }

    eglDestroySurface(mDisplay, mEglSurface);
    eglDestroyContext(mDisplay, mContext);
    ANativeWindow_release(pWindow);
}