//
// Created by xulinkai on 2021/10/27.
//

#include "YUVRenderer.h"

YUVRenderer* pRenderer;

extern "C"
JNIEXPORT void JNICALL
Java_com_github_opengles_1android_examples_yuv_YUVRenderer_initRenderer(JNIEnv *env,
                                                                          jobject thiz,
                                                                          jobject surface,
                                                                          jint surface_width,
                                                                          jint surface_height,
                                                                          jobject assetManager) {
    InitUtil(env, assetManager);
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    pRenderer = new YUVRenderer(window, surface_width, surface_height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_opengles_1android_examples_yuv_YUVRenderer_shutDown(JNIEnv *env, jobject thiz) {
    //TODO pRender被释放导致泄漏
    if (pRenderer) {
        pRenderer->ShutDown();
        delete(pRenderer);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_github_opengles_1android_examples_yuv_YUVRenderer_onSurfaceChanged(JNIEnv *env,
                                                                            jobject thiz,
                                                                            jint width,
                                                                            jint height) {
    if (pRenderer) {
        pRenderer->OnSurfaceChanged(width, height);
    }
}