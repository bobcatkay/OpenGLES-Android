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

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "OpenGLES-Android", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "OpenGLES-Android", __VA_ARGS__)

const EGLint IMAGE_KHR_ATTR[3] = {EGL_IMAGE_PRESERVED_KHR, EGL_TRUE, EGL_NONE};

void InitUtil(JNIEnv* env, jobject assetManager);

std::string ReadFileFromAssets(const char* fileName);

uint8_t* ReadDataFromAssets(const char* fileName, int& size);

void InitQuadVAO(GLuint& vao);

void InitContext(EGLDisplay& display, ANativeWindow* window, EGLSurface& surface, EGLContext& context);

void BindHardwareBuffer(GLuint texId, AHardwareBuffer* buffer, EGLDisplay& display);

#endif