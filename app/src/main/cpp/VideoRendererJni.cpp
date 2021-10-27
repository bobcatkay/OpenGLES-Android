//
// Created by xulinkai on 2021/10/27.
//

#include "VideoRenderer.h"

VideoRenderer* pRenderer;
VideoDecoder *pDecoder;

extern "C"
JNIEXPORT void JNICALL
Java_com_github_opengles_1android_examples_video_VideoRenderer_initRenderer(JNIEnv *env,
                                                                            jobject thiz,
                                                                            jobject surface,
                                                                            jint surface_width,
                                                                            jint surface_height,
                                                                            jobject assetManager) {
    InitUtil(env, assetManager);
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    pRenderer = new VideoRenderer(window, surface_width, surface_height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_opengles_1android_examples_video_VideoRenderer_onDrawFrame(JNIEnv *env,
                                                                           jobject thiz) {
    if (nullptr != pRenderer) {
        pRenderer->OnDrawFrame();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_opengles_1android_examples_video_VideoRenderer_release(JNIEnv *env, jobject thiz) {
    if (nullptr != pRenderer) {
        pRenderer->Release();
        delete(pRenderer);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_opengles_1android_examples_video_VideoRenderer_initVideoDecoder(JNIEnv *env,
                                                                                jobject thiz) {
    pDecoder = new VideoDecoder("test.mp4");
}