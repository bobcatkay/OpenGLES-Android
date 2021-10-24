//
// Created by xulinkai on 2021/10/24.
//
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

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "render", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "render", __VA_ARGS__)


void JniInit(JNIEnv* env, jobject assetManager);

std::string ReadFileFromAssets(const char* fileName);

void BindHardwareBuffer(GLuint texId, AHardwareBuffer* buffer);