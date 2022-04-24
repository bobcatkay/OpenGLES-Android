//
// Created by xulinkai on 2021/10/24.
//

#include "CameraRenderer.h"

CameraRenderer::CameraRenderer(ANativeWindow *window, int surfaceWidth, int surfaceHeight) {
    pWindow = window;
    mWindowWidth = surfaceWidth;
    mWindowHeight = surfaceHeight;

    Init();
}

void CameraRenderer::Init() {
    mProjectionMatrix = glm::mat4(1.0f);
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
    const std::string fragCode = ReadFileFromAssets("texture2d_oes.frag");
    pShader = new Shader(vertexCode.c_str(), fragCode.c_str());

    glGenTextures(1, &mTexId);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, mTexId);

    InitQuadVAO(mVAO);
}

void CameraRenderer::UpdateTexture(AHardwareBuffer* buffer, int width, int height) {
    if (nullptr == buffer) {
        return;
    }

    if ((mLastBufferWidth != width) || (mLastBufferHeight != height)) {
        mLastBufferWidth = width;
        mLastBufferHeight = height;
        mTransformMatrix = glm::mat4(1.0f);
        float scaleY = 1.0f;
        float scaleX = 1.0f;

        if (mWindowHeight > mWindowWidth) {
            scaleY = (float) height / width;
        } else {
            scaleX = (float) width / height;
        }

        LOGD("UpdateTexture, mWindowWidth: %d, mWindowHeight: %d, width: %d, height: %d, scaleX: %.1f, scaleY: %.1f.",
             mWindowWidth, mWindowHeight, width, height, scaleX, scaleY);

        mTransformMatrix = glm::scale(mTransformMatrix, glm::vec3(scaleX, scaleY, 1.0f));
        mTransformMatrix = glm::rotate(mTransformMatrix, glm::radians(270.0f), glm::vec3(0, 0, 1.0f));
    }

    BindHardwareBuffer(mTexId, buffer, mDisplay);
}

void CameraRenderer::OnDrawFrame(AHardwareBuffer *buffer, int width, int height) {
    glViewport(0, 0, mWindowWidth, mWindowHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f,0.0f,0.0f,1.0f);

    UpdateTexture(buffer, width, height);

    pShader->UseProgram();
    pShader->SetInt("uTexture", 0);
    pShader->SetMat4("uProjection", mProjectionMatrix);
    pShader->SetMat4("uTransform", mTransformMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, mTexId);

    glBindVertexArray(mVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, VERTEX_COUNT);

    eglSwapBuffers(mDisplay, mEglSurface);
}

void CameraRenderer::Release() {
    glDeleteBuffers(1, &mVAO);
    glDeleteTextures(1, &mTexId);

    if (nullptr != pShader) {
        delete(pShader);
    }

    eglDestroySurface(mDisplay, mEglSurface);
    eglDestroyContext(mDisplay, mContext);
    ANativeWindow_release(pWindow);
}

