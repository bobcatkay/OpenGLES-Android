//
// Created by xulinkai on 2021/10/24.
//

#ifndef UTIL_H
#define UTIL_H

#include <jni.h>
#include <string>
#include <android/log.h>
#define EGL_EGLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/hardware_buffer.h>
#include <android/hardware_buffer_jni.h>
#include <media/NdkImageReader.h>

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "camera_renderer", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "camera_renderer", __VA_ARGS__)

const EGLint IMAGE_KHR_ATTR[3] = {EGL_IMAGE_PRESERVED_KHR, EGL_TRUE, EGL_NONE};

//struct Texture {
//
//    GLuint id;
//    GLint location;
//    GLint width;
//    GLint height;
//    GLint format;
//
//    const char* ToString() {
//        char mPrintString[128];
//        sprintf(mPrintString, "{id: %d, location: %d, width: %d, height: %d, format: %d}", id, location, width, height, format);
//
//        return mPrintString;
//    }
//
////private:
////    char mPrintString[128];
//};

void InitUtil(JNIEnv* env, jobject assetManager);

std::string ReadFileFromAssets(const char* fileName);

void GetAssetPath(char path[], const char* fileName);

uint8_t* ReadDataFromAssets(const char* fileName, int& size);

void InitQuadVAO(GLuint& vao);

void InitContext(EGLDisplay& display, ANativeWindow* window, EGLSurface& surface, EGLContext& context);

void BindHardwareBuffer(GLuint texId, AHardwareBuffer* buffer, EGLDisplay& display);

#endif