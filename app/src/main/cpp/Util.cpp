//
// Created by xulinkai on 2021/10/24.
//
#include "Util.h"

static AAssetManager* pAssetManager;

void InitUtil(JNIEnv* env, jobject assetManager) {
    pAssetManager = AAssetManager_fromJava(env, assetManager);
}

void GetAssetPath(char path[], const char* fileName) {
    AAsset *asset = AAssetManager_open(pAssetManager, fileName, AASSET_MODE_RANDOM);
    off64_t offset, length;
    int fd = AAsset_openFileDescriptor64(asset, &offset, &length);
    sprintf(path, "/proc/self/fd/%d", fd);

    AAsset_close(asset);
}

uint8_t* ReadDataFromAssets(const char* fileName, int& size) {
    
    AAsset *pAsset = AAssetManager_open(pAssetManager, fileName, AASSET_MODE_RANDOM);
    size = AAsset_getLength(pAsset);
    LOGD("ReadDataFromAssets, size: %d", size);
    uint8_t *buf = new uint8_t[size];
    int ret = AAsset_read(pAsset, buf, size);
    AAsset_close(pAsset);
    LOGD("ReadDataFromAssets, ret: %d", ret);
    return buf;
}

std::string ReadFileFromAssets(const char* fileName) {
    LOGD("ReadFileFromAssets, file: %s.", fileName);

    std::string content;

    if (nullptr == pAssetManager) {
        LOGE("ReadFileFromAssets, %s","AAssetManager==NULL");
        return nullptr;
    }

    /*获取文件名并打开*/
    AAsset* asset = AAssetManager_open(pAssetManager, fileName,AASSET_MODE_UNKNOWN);

    if (nullptr == asset) {
        LOGE("ReadFileFromAssets, %s","asset==NULL");
        return nullptr;
    }

    /*获取文件大小*/
    off_t bufferSize = AAsset_getLength(asset);
    char *buffer = (char *) malloc(bufferSize+1);
    buffer[bufferSize] = 0;
    int numBytesRead = AAsset_read(asset, buffer, bufferSize);

    if (numBytesRead > 0) {
        content = std::string(buffer);
    }

    LOGD("ReadFileFromAssets, %s",buffer);
    free(buffer);
    /*关闭文件*/
    AAsset_close(asset);

    return content;
}

void InitQuadVAO(GLuint& vao) {
    float vertices[] = {
            // positions           // texture coords
            1.0f,  1.0f, 0.0f,     1.0f, 0.0f, // top right
            1.0f, -1.0f, 0.0f,     1.0f, 1.0f, // bottom right
            -1.0f, -1.0f, 0.0f,    0.0f, 1.0f, // bottom left
            -1.0f,  1.0f, 0.0f,    0.0f, 0.0f  // top left
    };
    unsigned int indices[] = {
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
    };
    unsigned int VBO, EBO;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vao);
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

void InitContext(EGLDisplay& display, ANativeWindow* window, EGLSurface& surface, EGLContext& context) {
    EGLint majorVersion;
    EGLint minorVersion;

    // Create and initialize EGLDisplay
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    if (EGL_NO_DISPLAY == display) {
        LOGE("InitContext, eglGetDisplay failed!");
        return;
    }

    if (!eglInitialize(display, &majorVersion, &minorVersion)) {
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

    if (!eglChooseConfig(display, configAttribs, &config, 1, &configNum)) {
        EGLint error = eglGetError();
        LOGE("InitContext, eglChooseConfig failed! error: %d.", error);
        return;
    }

    surface = eglCreateWindowSurface(display, config, window, nullptr);

    if (EGL_NO_SURFACE == surface) {
        EGLint error = eglGetError();
        LOGE("InitContext, eglCreateWindowSurface failed! error: %d.", error);
        return;
    }

    const EGLint ctxAttr[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctxAttr);

    if (EGL_NO_CONTEXT == context) {
        EGLint error = eglGetError();
        LOGE("InitContext, eglCreateContext failed! error: %d.", error);
        return;
    }

    if (!eglMakeCurrent(display, surface, surface, context)) {
        EGLint error = eglGetError();
        LOGE("InitContext, eglMakeCurrent failed! error: %d.", error);
        return;
    }
}

void BindHardwareBuffer(GLuint texId, AHardwareBuffer* buffer, EGLDisplay& display) {
    EGLClientBuffer clientBuffer = eglGetNativeClientBufferANDROID(buffer);

    if (nullptr == clientBuffer) {
        LOGE("BindHardwareBuffer, clientBuffer is null.");
        return;
    }

    // Create EGLImage from EGLClientBuffer.
    EGLImageKHR image = eglCreateImageKHR(display, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID, clientBuffer, IMAGE_KHR_ATTR);

    if (nullptr == image) {
        LOGE("BindHardwareBuffer, image is null.");
        return;
    }

    // Create OpenGL texture from the EGLImage.
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texId);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, image);
}