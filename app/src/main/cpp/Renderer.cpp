//
// Created by xulinkai on 2021/10/24.
//

#include "Renderer.h"
#include <thread>

static std::mutex sMutex;

Renderer::Renderer(ANativeWindow *window, int surfaceWidth, int surfaceHeight) {
    pWindow = window;
    mWindowWidth = surfaceWidth;
    mWindowHeight = surfaceHeight;
    InitContext();
    Run();
}

Renderer::~Renderer() {
    eglDestroySurface(mDisplay, mEglSurface);
    eglDestroyContext(mDisplay, mContext);
    ANativeWindow_release(pWindow);
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

void Renderer::Run() {
    const std::string vertexCode = ReadFileFromAssets("common.vert");
    const std::string fragCode = ReadFileFromAssets("texture2d_oes.frag");
    pShader = new Shader(vertexCode.c_str(), fragCode.c_str());

    glGenTextures(1, &mTexId);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, mTexId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    InitVertex();

    while (!mbShutDown) {
        glViewport(0, 0, mWindowWidth, mWindowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(1.0f,1.0f,0.0f,1.0f);

        onDraw();

        eglSwapBuffers(mDisplay, mEglSurface);
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

void Renderer::onDraw() {
    LOGD("onDraw");

//    std::lock_guard<std::mutex> lock(sMutex);
//    if (!mBufferList.empty()) {
    if (nullptr != pBuffer) {
        LOGD("onDraw, texId: %d.", mTexId);

//        AHardwareBuffer *pBuffer = mBufferList.back();
//        mBufferList.pop_back();
        BindHardwareBuffer(mTexId, pBuffer);

        LOGD("onDraw, texId: %d.", mTexId);

        pBuffer = nullptr;
    }

    pShader->UseProgram();
    pShader->SetInt("uTexture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, mTexId);

    glBindVertexArray(mVAO);
    glDrawElements(GL_TRIANGLES, mVertexCount, GL_UNSIGNED_INT, 0);
}

void Renderer::ShutDown() {
    mbShutDown = true;
}

void Renderer::AddBuffer(AHardwareBuffer* buffer) {
//    std::lock_guard<std::mutex> lock(sMutex);
//    mBufferList.push_front(buffer);
    pBuffer = buffer;
}

