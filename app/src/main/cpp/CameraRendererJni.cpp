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
    if (nullptr != pRenderer) {
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

    if (nullptr != pBuffer) {
        if (nullptr != pRenderer) {
            pRenderer->OnDrawFrame(pBuffer, width, height);
        }
    } else {
        LOGE("addBuffer, pBuffer is null.");
    }

}