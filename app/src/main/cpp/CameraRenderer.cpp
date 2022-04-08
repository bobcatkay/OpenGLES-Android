//
// Created by xulinkai on 2021/10/24.
//

#include "CameraRenderer.h"
#include "stb_image_write.h"
#include "stb_image.h"

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
    eglSwapInterval(mDisplay, 1);

//    int width, height, channels;
//    const char* path = "/storage/emulated/0/Android/data/com.github.opengles_android/files/Pictures/test2.jpg";
//    unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
//
//    if (!data) {
//        LOGE("InitTexture, Failed to load image at %s.", path);
//    }

    //mLocalTexture = Texture::GenTexture(width, height, data, 0);


    for (int i = 0; i < DUMP_SIZE; i++) {
        Texture *pTexture = Texture::GenTexture(mWindowWidth, mWindowHeight, nullptr, 0);
        mTexColorBuffer.push_back(pTexture);
        GLuint fbo;
        glGenFramebuffers(1, &fbo);
        mFBO.push_back(fbo);
    }
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
    pShader->SetInt("mRotation", mRotation);
    pShader->SetMat4("uProjection", mProjectionMatrix);
    pShader->SetMat4("uTransform", mTransformMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, mTexId);
    glBindVertexArray(mVAO);
    glDrawElements(GL_TRIANGLES, VERTEX_COUNT, GL_UNSIGNED_INT, 0);

    if (mbFlag) {
        auto t = mTexColorBuffer[mbFlag-1];
        auto fbo = mFBO[mbFlag-1];
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(1.0f, 1.0f, 1.0f,1.0f);
        glViewport(0, 0, t->width, t->height);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, t->id, 0);
        glDrawElements(GL_TRIANGLES, VERTEX_COUNT, GL_UNSIGNED_INT, 0);

        mbFlag++;

        if (mbFlag == (mTexColorBuffer.size() + 1)) {
            mbFlag = 0;

            for (int i = 0; i < mTexColorBuffer.size(); i++) {
                glBindFramebuffer(GL_FRAMEBUFFER, mFBO[i]);
                char* file = new char[16];
                sprintf(file, "%d.jpg", i);
                ReadTexture(file, t->width, t->height);
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    eglSwapBuffers(mDisplay, mEglSurface);
}

void CameraRenderer::Release() {
    glDeleteBuffers(1, &mVAO);
    glDeleteTextures(1, &mTexId);

    for (auto& t: mTexColorBuffer) {
        delete(t);
    }

    for (auto& f: mFBO) {
        glDeleteBuffers(1, &f);
    }

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
            mProjectionMatrix = glm::ortho(-1.0f, 1.0f, -aspectRatio, aspectRatio, -1.0f, 1.0f);
        } else {
            mProjectionMatrix = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f);
        }
    } else {
        if (mWindowWidth > mWindowHeight) {
            mProjectionMatrix = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f);
        } else {
            mProjectionMatrix = glm::ortho(-1.0f, 1.0f, -aspectRatio, aspectRatio, -1.0f, 1.0f);
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
            scaleX = (float) mLastBufferWidth / mLastBufferHeight;
        } else {
            scaleY = (float) mLastBufferHeight / mLastBufferWidth;
        }
    } else {
        if (mWindowHeight > mWindowWidth) {
            scaleY = (float) mLastBufferWidth / mLastBufferHeight;
        } else {
            scaleX = (float) mLastBufferHeight / mLastBufferWidth;
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

void CameraRenderer::ReadTexture(const char* file, int width, int height) {
    GLubyte* pixels = new GLubyte[width * height * 3];
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    char* path = new char[128];
    sprintf(path, "%s/%s", mStoreDir, file);
    stbi_flip_vertically_on_write(true);
    stbi_write_jpg(path, width, height, 3, pixels, 100);
    delete[] pixels;
}

void CameraRenderer::SetTextureStoreDir(const char *path) {
    LOGD("SetTextureStoreDir, path: %s.", path);
    strcpy(mStoreDir, path);
}

void CameraRenderer::BeginDump() {
    mbFlag = 1;
}

