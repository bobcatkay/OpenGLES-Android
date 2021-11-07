//
// Created by xulinkai on 2021/10/27.
//

#include "YUVRenderer.h"

YUVRenderer::YUVRenderer(ANativeWindow *window, int surfaceWidth, int surfaceHeight) {
    pWindow = window;
    mWindowWidth = surfaceWidth;
    mWindowHeight = surfaceHeight;

    Init();
}

void YUVRenderer::Init() {


    InitContext(mDisplay, pWindow, mEglSurface, mContext);

    const std::string vertexCode = ReadFileFromAssets("common.vert");
    const std::string fragCode = ReadFileFromAssets("texture2d_yuv420p.frag");
    pShader = new Shader(vertexCode.c_str(), fragCode.c_str());

    InitQuadVAO(mVAO);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    pShader->UseProgram();


    int size = 0;
    uint8_t *data = ReadDataFromAssets("test.yuv", size);
    LOGD("YUVRenderer::Init, size: %d", size);
    int yDataSize = TEXTURE_WIDTH * TEXTURE_HEIGHT;
    int uDataSize = yDataSize / 4;
    int vDataSize = yDataSize / 4;
    uint8_t* yData = new uint8_t[yDataSize];
    uint8_t* uData = new uint8_t[uDataSize];
    uint8_t* vData = new uint8_t[vDataSize];

    memcpy(yData, &data[0], yDataSize);
    memcpy(uData, &data[yDataSize], uDataSize);
    memcpy(vData, &data[yDataSize + uDataSize], vDataSize);

    mTexture[0] = Texture::GenSingleChannelTexture(TEXTURE_WIDTH, TEXTURE_HEIGHT, yData, 0);
    mTexture[1] = Texture::GenSingleChannelTexture(TEXTURE_WIDTH / 2, TEXTURE_HEIGHT / 2, uData, 1);
    mTexture[2] = Texture::GenSingleChannelTexture(TEXTURE_WIDTH / 2, TEXTURE_HEIGHT / 2, vData, 2);

//    mTexture[0]->UpdateData(yData);
//    mTexture[1]->UpdateData(uData);
//    mTexture[2]->UpdateData(vData);
    delete[](data);
    delete[](yData);
    delete[](uData);
    delete[](vData);

    pShader->SetInt("yTex", mTexture[0]->location);
    pShader->SetInt("uTex", mTexture[1]->location);
    pShader->SetInt("vTex", mTexture[2]->location);

    InitMatrix();

    while (!mbShutDown) {
        OnDrawFrame();
    }

    Release();
}

void YUVRenderer::OnDrawFrame() {
    glViewport(0, 0, mWindowWidth, mWindowHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(1.0f,1.0f,1.0f,1.0f);

    pShader->UseProgram();
    pShader->SetMat4("uProjection", mProjectionMatrix);
    pShader->SetMat4("uTransform", mTransformMatrix);

    for (int i=0; i<3; i++) {
        if (mTexture[i]) {
            mTexture[i]->ActiveTexture();
        }
    }

    glBindVertexArray(mVAO);
    glDrawElements(GL_TRIANGLES, VERTEX_COUNT, GL_UNSIGNED_INT, 0);

    eglSwapBuffers(mDisplay, mEglSurface);
}

void YUVRenderer::Release() {
    eglDestroySurface(mDisplay, mEglSurface);
    eglDestroyContext(mDisplay, mContext);
    ANativeWindow_release(pWindow);

    FreeResources();

    if (pShader) {
        delete(pShader);
    }
}

void YUVRenderer::FreeResources() {
    for (int i = 0; i < 3;i++) {
        delete(mTexture[i]);
    }

    glDeleteBuffers(1, &mVAO);
}

void YUVRenderer::UpdateTexture() {

}


void YUVRenderer::InitYUVTexture() {
    LOGD("InitYUVTexture");

}

void YUVRenderer::ShutDown() {
    mbShutDown = true;
}

void YUVRenderer::OnSurfaceChanged(int width, int height) {
    mWindowWidth = width;
    mWindowHeight = height;

    InitMatrix();
}

void YUVRenderer::InitMatrix() {
    float aspectRatio = mWindowWidth > mWindowHeight ?
                        (float) mWindowWidth / (float) mWindowHeight :
                        (float) mWindowHeight / (float) mWindowWidth;

    if (mWindowWidth > mWindowHeight) {
        mProjectionMatrix = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f);
    } else {
        mProjectionMatrix = glm::ortho(-1.0f, 1.0f, -aspectRatio, aspectRatio, -1.0f, 1.0f);
    }

    mTransformMatrix = glm::mat4(1.0f);
    float texRatio = (float) TEXTURE_HEIGHT / TEXTURE_WIDTH;
    float screenRatio = (float) mWindowHeight / mWindowWidth;
    float scaleY = 1.0f;
    float scaleX = 1.0f;

    if (screenRatio > texRatio) {
        scaleY = (float) mWindowWidth * texRatio / mWindowWidth;
    } else {
        scaleX = (float) mWindowHeight / texRatio / mWindowHeight;
    }

    mTransformMatrix = glm::scale(mTransformMatrix, glm::vec3(scaleX, scaleY, 1.0f));
}
