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
    UpdateProjection();

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
        UpdateTransform();
    }

    BindHardwareBuffer(mTexId, buffer, mDisplay);
}

void CameraRenderer::OnDrawFrame(AHardwareBuffer *buffer, int width, int height) {
    glViewport(0, 0, mWindowWidth, mWindowHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(1.0f, 1.0f, 1.0f,1.0f);

    UpdateTexture(buffer, width, height);

    pShader->UseProgram();
    pShader->SetInt("uTexture", 0);
    pShader->SetMat4("uProjection", mProjectionMatrix);
    pShader->SetMat4("uTransform", mTransformMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, mTexId);

    glBindVertexArray(mVAO);
    glDrawElements(GL_TRIANGLES, VERTEX_COUNT, GL_UNSIGNED_INT, 0);

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

void CameraRenderer::UpdateProjection() {
    mProjectionMatrix = glm::mat4(1.0f);
    float aspectRatio = mWindowWidth > mWindowHeight ?
                        (float) mWindowWidth / (float) mWindowHeight :
                        (float) mWindowHeight / (float) mWindowWidth;

    if (0 == mRotation) {
        if (mWindowWidth > mWindowHeight) {
            mProjectionMatrix = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f);
        } else {
            mProjectionMatrix = glm::ortho(-1.0f, 1.0f, -aspectRatio, aspectRatio, -1.0f, 1.0f);
        }
    } else {
        if (mWindowWidth > mWindowHeight) {
            mProjectionMatrix = glm::ortho(-1.0f, 1.0f, -aspectRatio, aspectRatio, -1.0f, 1.0f);
        } else {
            mProjectionMatrix = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f);
        }
    }
}

void CameraRenderer::OnSurfaceChanged(int width, int height, int rotation) {
    LOGD("OnSurfaceChanged, width: %d, height: %d, rotation: %d", width, height, rotation);

    mWindowWidth = width;
    mWindowHeight = height;
    mRotation = rotation;
    UpdateProjection();
    UpdateTransform();
}

void CameraRenderer::UpdateTransform() {
    mTransformMatrix = glm::mat4(1.0f);
    float scaleY = 1.0f;
    float scaleX = 1.0f;

    if (0 == mRotation) {
        if (mWindowHeight > mWindowWidth) {
            scaleY = (float) mLastBufferHeight / mLastBufferWidth;
        } else {
            scaleX = (float) mLastBufferWidth / mLastBufferHeight;
        }
    } else {
        if (mWindowHeight > mWindowWidth) {
            scaleX = (float) mLastBufferHeight / mLastBufferWidth;
        } else {
            scaleY = (float) mLastBufferWidth / mLastBufferHeight;
        }
    }

    LOGD("UpdateTexture, mWindowWidth: %d, mWindowHeight: %d, width: %d, height: %d, scaleX: %.1f, scaleY: %.1f.",
         mWindowWidth, mWindowHeight, mLastBufferWidth, mLastBufferHeight, scaleX, scaleY);

    mTransformMatrix = glm::scale(mTransformMatrix, glm::vec3(scaleX, scaleY, 1.0f));

    if (mRotation == 0) {
        mTransformMatrix = glm::rotate(mTransformMatrix, glm::radians(270.0f), glm::vec3(0, 0, 1.0f));
    } else if (mRotation == 3) {
        mTransformMatrix = glm::rotate(mTransformMatrix, glm::radians(180.0f), glm::vec3(0, 0, 1.0f));
    }
}

