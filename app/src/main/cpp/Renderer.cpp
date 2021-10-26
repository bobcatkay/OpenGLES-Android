//
// Created by xulinkai on 2021/10/24.
//

#include "Renderer.h"

Renderer::Renderer(ANativeWindow *window, int surfaceWidth, int surfaceHeight) {
    pWindow = window;
    mWindowWidth = surfaceWidth;
    mWindowHeight = surfaceHeight;

    Init();
}

Renderer::~Renderer() {

}

void Renderer::InitContext() {
    EGLint majorVersion;
    EGLint minorVersion;

    // Create and initialize EGLDisplay
    mDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    if (EGL_NO_DISPLAY == mDisplay) {
        LOGE("InitContext, eglGetDisplay failed!");
        return;
    }

    if (!eglInitialize(mDisplay, &majorVersion, &minorVersion)) {
        LOGE("InitContext, eglInitialize failed!");
        return;
    }

    // Config and create EGLSurface
    EGLConfig config;
    EGLint configNum;
    EGLint configAttribs[] = {
            EGL_RENDERABLE_TYPE, EGL_WINDOW_BIT,
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE
    };

    if (!eglChooseConfig(mDisplay, configAttribs, &config, 1, &configNum)) {
        EGLint error = eglGetError();
        LOGE("InitContext, eglChooseConfig failed! error: %d.", error);
        return;
    }

    mEglSurface = eglCreateWindowSurface(mDisplay, config, pWindow, nullptr);

    if (EGL_NO_SURFACE == mEglSurface) {
        EGLint error = eglGetError();
        LOGE("InitContext, eglCreateWindowSurface failed! error: %d.", error);
        return;
    }

    const EGLint ctxAttr[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    mContext = eglCreateContext(mDisplay, config, EGL_NO_CONTEXT, ctxAttr);

    if (EGL_NO_CONTEXT == mContext) {
        EGLint error = eglGetError();
        LOGE("InitContext, eglCreateContext failed! error: %d.", error);
        return;
    }

    if (!eglMakeCurrent(mDisplay, mEglSurface, mEglSurface, mContext)) {
        EGLint error = eglGetError();
        LOGE("InitContext, eglMakeCurrent failed! error: %d.", error);
        return;
    }
}

void Renderer::InitVertex() {
    float vertices[] = {
            // positions           // texture coords
            1.0f,  1.0f, 0.0f,     1.0f, 1.0f, // top right
            1.0f, -1.0f, 0.0f,     1.0f, 0.0f, // bottom right
            -1.0f, -1.0f, 0.0f,    0.0f, 0.0f, // bottom left
            -1.0f,  1.0f, 0.0f,    0.0f, 1.0f  // top left
    };
    unsigned int indices[] = {
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
    };
    unsigned int VBO, EBO;
    glGenVertexArrays(1, &mVAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(mVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void Renderer::BindHardwareBuffer(GLuint texId, AHardwareBuffer* buffer) {
    EGLClientBuffer clientBuffer = eglGetNativeClientBufferANDROID(buffer);

    if (nullptr == clientBuffer) {
        LOGE("BindHardwareBuffer, clientBuffer is null.");
        return;
    }

    // Create EGLImage from EGLClientBuffer.
    EGLImageKHR image = eglCreateImageKHR(mDisplay, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID, clientBuffer, IMAGE_KHR_ATTR);

    if (nullptr == image) {
        LOGE("BindHardwareBuffer, image is null.");
        return;
    }

    // Create OpenGL texture from the EGLImage.
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texId);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, image);
}

void Renderer::Init() {
    float aspectRatio = mWindowWidth > mWindowHeight ?
                        (float) mWindowWidth / (float) mWindowHeight :
                        (float) mWindowHeight / (float) mWindowWidth;

    if (mWindowWidth < mWindowHeight) {
        mProjectionMatrix = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f);
    } else {
        mProjectionMatrix = glm::ortho(-1.0f, 1.0f, -aspectRatio, aspectRatio, -1.0f, 1.0f);
    }

    InitContext();

    const std::string vertexCode = ReadFileFromAssets("common.vert");
    const std::string fragCode = ReadFileFromAssets("texture2d_oes.frag");
    pShader = new Shader(vertexCode.c_str(), fragCode.c_str());

    glGenTextures(1, &mTexId);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, mTexId);

    InitVertex();
}

void Renderer::UpdateTexture(AHardwareBuffer* buffer, int width, int height) {
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

        LOGD("UpdateTexture, mWindowWidth: %d, mWindowHeight: %d, width: %d, height: %d, scaleY: %.1f.", mWindowWidth,
             mWindowHeight, width, height, scaleY);

        mTransformMatrix = glm::scale(mTransformMatrix, glm::vec3(scaleX, scaleY, 1.0f));
        mTransformMatrix = glm::rotate(mTransformMatrix, glm::radians(270.0f), glm::vec3(0, 0, 1.0f));

        // Left-Right mirror
        mTransformMatrix = glm::scale(mTransformMatrix, glm::vec3(1.0f, -1.0f, 1.0f));
    }

    if (nullptr != pLastBuffer) {
        AHardwareBuffer_release(pLastBuffer);
    }

    BindHardwareBuffer(mTexId, buffer);
    pLastBuffer = buffer;
}

void Renderer::onDrawFrame(AHardwareBuffer *buffer, int width, int height) {
    glViewport(0, 0, mWindowWidth, mWindowHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(1.0f,1.0f,0.0f,1.0f);

    UpdateTexture(buffer, width, height);

    pShader->UseProgram();
    pShader->SetInt("uTexture", 0);
    pShader->SetMat4("uProjection", mProjectionMatrix);
    pShader->SetMat4("uTransform", mTransformMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, mTexId);

    glBindVertexArray(mVAO);
    glDrawElements(GL_TRIANGLES, mVertexCount, GL_UNSIGNED_INT, 0);

    eglSwapBuffers(mDisplay, mEglSurface);
}

void Renderer::Release() {
    eglDestroySurface(mDisplay, mEglSurface);
    eglDestroyContext(mDisplay, mContext);
    ANativeWindow_release(pWindow);

    glDeleteBuffers(1, &mVAO);
    glDeleteTextures(1, &mTexId);

    if (nullptr != pShader) {
        delete(pShader);
    }

    if (nullptr != pLastBuffer) {
        AHardwareBuffer_release(pLastBuffer);
    }
}

