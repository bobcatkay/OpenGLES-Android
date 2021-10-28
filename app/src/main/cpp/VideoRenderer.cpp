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
    //StartDecoder(videoFileFD);
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

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    pShader->UseProgram();
    int width = 1920;
    int height = 1080;


    int size = 0;
    uint8_t *data = ReadDataFromAssets("test.yuv", size);
    LOGD("VideoRenderer::Init, size: %d", size);
    int yDataSize = width * height;
    int uDataSize = yDataSize / 4;
    int vDataSize = yDataSize / 4;
    uint8_t* yData = new uint8_t[yDataSize];
    uint8_t* uData = new uint8_t[uDataSize];
    uint8_t* vData = new uint8_t[vDataSize];

    memcpy(yData, &data[0], yDataSize);
    memcpy(uData, &data[yDataSize], uDataSize);
    memcpy(vData, &data[yDataSize + uDataSize], vDataSize);

    mTexture[0] = Texture::GenSingleChannelTexture(width, height, yData, 0);
    mTexture[1] = Texture::GenSingleChannelTexture(width / 2, height / 2, uData, 1);
    mTexture[2] = Texture::GenSingleChannelTexture(width / 2, height / 2, vData, 2);
    pShader->SetInt("yTex", mTexture[0]->location);
    pShader->SetInt("uTex", mTexture[1]->location);
    pShader->SetInt("vTex", mTexture[2]->location);
    pShader->SetMat3("uBT709Matrix", MATRIX_BT709);
}

void VideoRenderer::OnDrawFrame() {
    glViewport(0, 0, mWindowWidth, mWindowHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(1.0f,1.0f,1.0f,1.0f);

    pShader->UseProgram();
    pShader->SetMat4("uProjection", mProjectionMatrix);
    pShader->SetMat4("uTransform", mTransformMatrix);

    //UpdateTexture();

    for (int i=0; i<3; i++) {
        if (mTexture[i]) {
            mTexture[i]->ActiveTexture();
        }
    }

    glBindVertexArray(mVAO);
    glDrawElements(GL_TRIANGLES, VERTEX_COUNT, GL_UNSIGNED_INT, 0);

    eglSwapBuffers(mDisplay, mEglSurface);
}

void VideoRenderer::Release() {
    if (nullptr != pVideoDecoder) {
        pVideoDecoder->Shutdown();
        //delete(pVideoDecoder);
    }

    eglDestroySurface(mDisplay, mEglSurface);
    eglDestroyContext(mDisplay, mContext);
    ANativeWindow_release(pWindow);

    FreeResources();

    if (nullptr != pShader) {
        delete(pShader);
    }
}

void VideoRenderer::FreeResources() {
    for (int i = 0; i < 3;i++) {
        delete(mTexture[i]);
    }

    glDeleteBuffers(1, &mVAO);
}

void VideoRenderer::StartDecoder(int videoFileFD) {
    std::lock_guard<std::mutex> lock(sMutex);
    if (!mFrame) {
        mFrame = av_frame_alloc();
    }
    //创建视频解码器
//    pVideoDecoder = new VideoDecoder(videoFileFD);
//    pVideoDecoder->GetVideoSize(mVideoWidth, mVideoHeight);
//    bVideoSizeInit = true;
//    auto callback = std::bind(&VideoRenderer::VideoDecodeCallback,this,std::placeholders::_1);
//    pVideoDecoder->Start(callback);

    LOGD("StartDecoder, mVideoWidht: %d, mVideoHeight: %d", mVideoWidth, mVideoHeight);
}

void VideoRenderer::VideoDecodeCallback(AVFrame *frame) {
    LOGD("VideoDecodeCallback");
    sMutex.lock();
    if (!mFrame) {
        sMutex.unlock();
        return;
    }

    if (frame->width != mVideoWidth) {
        sMutex.unlock();
        return;
    }

    av_frame_unref(mFrame);
    av_frame_move_ref(mFrame, frame);
    sMutex.unlock();
    LOGD("VideoDecodeCallback, Exit.");
}

void VideoRenderer::UpdateTexture() {
    LOGD("UpdateTexture, mVideoWidth: %d, mVideoHeight: %d", mVideoWidth, mVideoHeight);

    if (!mVideoWidth){
        return;
    }

    if (!mTexture[0]) {
        InitYUVTexture();
    }

    sMutex.lock();
    if (!mTexture[0] || mTexture[0]->width != mFrame->width) {
        sMutex.unlock();
        return;
    }

    LOGD("UpdateTexture, format: %d", mFrame->format);

    if (mFrame->format >= 0) {
        for (int i = 0; i < 3; i++) {
            mTexture[i]->UpdateData(mFrame->data[i]);

            LOGD("UpdateTexture, textures: %s", mTexture[i]->ToString());
        }
    }

    sMutex.unlock();
}


void VideoRenderer::InitYUVTexture() {
    LOGD("InitYUVTexture");
    std::lock_guard<std::mutex> lock(sMutex);
    if (!mFrame || !mVideoWidth) {
        return;
    }

    mTexture[0] = Texture::GenSingleChannelTexture(mVideoWidth, mVideoHeight, nullptr, 1);
    mTexture[1] = Texture::GenSingleChannelTexture(mVideoWidth / 2, mVideoHeight / 2, nullptr, 2);
    mTexture[2] = Texture::GenSingleChannelTexture(mVideoWidth / 2, mVideoHeight / 2, nullptr, 3);

    pShader->UseProgram();
    pShader->SetInt("yTex", mTexture[0]->location);
    pShader->SetInt("uTex", mTexture[1]->location);
    pShader->SetInt("vTex", mTexture[2]->location);
    LOGD("InitYUVTexture, textures: [%s, %s, %s]", mTexture[0]->ToString(), mTexture[1]->ToString(), mTexture[2]->ToString());

    mTransformMatrix = glm::mat4(1.0f);
    float scaleY = 1.0f;
    float scaleX = 1.0f;

    if (mWindowHeight > mWindowWidth) {
        scaleY = (float) mVideoHeight / mVideoWidth;
    } else {
        scaleX = (float) mVideoWidth / mVideoHeight;
    }

    LOGD("UpdateTexture, mWindowWidth: %d, mWindowHeight: %d, mVideoWidth: %d, mVideoHeight: %d, scaleX: %.1f, scaleY: %.1f.",
         mWindowWidth, mWindowHeight, mVideoWidth, mVideoHeight, scaleX, scaleY);

    mTransformMatrix = glm::scale(mTransformMatrix, glm::vec3(scaleX, scaleY, 1.0f));
}
