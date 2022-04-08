//
// Created by xulinkai on 2021/10/24.
//

#include "CameraRenderer.h"

CameraRenderer *pRenderer;

extern "C"
JNIEXPORT void JNICALL
Java_com_github_opengles_1android_examples_camera_CameraRenderer_init(JNIEnv *env, jobject thiz,
                                                                      jobject surface,
                                                                      jint surface_width,
                                                                      jint surface_height,
                                                                      jobject assetManager) {
    InitUtil(env, assetManager);
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    pRenderer = new CameraRenderer(window, surface_width, surface_height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_opengles_1android_examples_camera_CameraRenderer_release(JNIEnv *env,
                                                                         jobject thiz) {
    if (pRenderer) {
        pRenderer->Release();
        delete(pRenderer);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_opengles_1android_examples_camera_CameraRenderer_onDrawFrame(JNIEnv *env,
                                                                             jobject thiz,
                                                                             jobject buffer,
                                                                             jint width,
                                                                             jint height) {
    AHardwareBuffer *pBuffer = AHardwareBuffer_fromHardwareBuffer(env, buffer);

    if (pBuffer) {
        if (pRenderer) {
            pRenderer->OnDrawFrame(pBuffer, width, height);
        }
    } else {
        LOGE("addBuffer, pBuffer is null.");
    }

}
extern "C"
JNIEXPORT void JNICALL
Java_com_github_opengles_1android_examples_camera_CameraRenderer_onSurfaceChanged(JNIEnv *env,
                                                                                  jobject thiz,
                                                                                  jint width,
                                                                                  jint height,
                                                                                  jint rotation) {
    if (pRenderer) {
        pRenderer->OnSurfaceChanged(width, height, rotation);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_github_opengles_1android_examples_camera_CameraRenderer_setTextureStoreDir(JNIEnv *env,
                                                                                    jobject thiz,
                                                                                    jstring dir_) {
    if (pRenderer) {
        const char *dir = env->GetStringUTFChars(dir_, nullptr);
        pRenderer->SetTextureStoreDir(dir);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_github_opengles_1android_examples_camera_CameraRenderer_beginDump(JNIEnv *env,
                                                                           jobject thiz) {
    if (pRenderer) {
        pRenderer->BeginDump();
    }
}