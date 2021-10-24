//
// Created by xulinkai on 2021/10/24.
//
#include "Util.h"

static JNIEnv* pJniEnv;
static AAssetManager* pAssetManager;
static EGLDisplay mEGLDisplay = nullptr;

void JniInit(JNIEnv* env, jobject assetManager) {
    pJniEnv = env;
    pAssetManager = AAssetManager_fromJava(env, assetManager);
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

static EGLImageKHR mEGLIMageKHR = EGL_NO_IMAGE_KHR;
void BindHardwareBuffer(GLuint texId, AHardwareBuffer* buffer) {
    LOGD("BindHardwareBuffer, texId: %d.", texId);

    if (nullptr == mEGLDisplay) {
        mEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    }

    if (EGL_NO_IMAGE_KHR != mEGLIMageKHR) {
        eglDestroyImageKHR(mEGLDisplay, mEGLIMageKHR);
        mEGLIMageKHR = EGL_NO_IMAGE_KHR;
    }

    EGLClientBuffer clientBuffer = eglGetNativeClientBufferANDROID(buffer);

    if (nullptr == clientBuffer) {
        LOGE("BindHardwareBuffer, clientBuffer is null.");
        return;
    }

    // Create EGLImage from EGLClientBuffer.
    EGLint imageAttrs[] = {EGL_IMAGE_PRESERVED_KHR, EGL_TRUE, EGL_NONE};
    EGLImageKHR image = eglCreateImageKHR(mEGLDisplay, EGL_NO_CONTEXT,
                                          EGL_NATIVE_BUFFER_ANDROID, clientBuffer, imageAttrs);

    if (nullptr == image) {
        LOGE("BindHardwareBuffer, image is null.");
        return;
    }
    // Create OpenGL texture from the EGLImage.
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texId);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, image);
    AHardwareBuffer_release(buffer);

    mEGLIMageKHR = image;
}