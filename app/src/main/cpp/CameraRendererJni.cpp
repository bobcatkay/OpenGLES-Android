//
// Created by xulinkai on 2021/10/24.
//

#include "Renderer.h"

Renderer *pRenderer;

extern "C"
JNIEXPORT void JNICALL
Java_com_github_opengles_1android_examples_camera_CameraRenderer_init(JNIEnv *env, jobject thiz,
                                                                      jobject surface,
                                                                      jint surface_width,
                                                                      jint surface_height,
                                                                      jobject assetManager) {
    JniInit(env, assetManager);
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    pRenderer = new Renderer(window, surface_width, surface_height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_opengles_1android_examples_camera_CameraRenderer_release(JNIEnv *env,
                                                                         jobject thiz) {
    if (nullptr != pRenderer) {
        pRenderer->Release();
        pRenderer = nullptr;
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

    if (nullptr != pBuffer) {
        if (nullptr != pRenderer) {
            pRenderer->onDrawFrame(pBuffer, width, height);
        }
    } else {
        LOGE("addBuffer, pBuffer is null.");
    }

}