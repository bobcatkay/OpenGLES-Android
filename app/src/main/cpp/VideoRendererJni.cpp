//
// Created by xulinkai on 2021/10/27.
//

#include "VideoRenderer.h"

VideoRenderer* pRenderer;
VideoDecoder* pVideoDecoder;

extern "C"
JNIEXPORT void JNICALL
Java_com_github_opengles_1android_examples_video_VideoView_initRenderer(JNIEnv *env,
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
Java_com_github_opengles_1android_examples_video_VideoView_onDrawFrame(JNIEnv *env,
                                                                       jobject thiz) {
    if (pRenderer) {
        pRenderer->OnDrawFrame();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_opengles_1android_examples_video_VideoView_initDecoder(JNIEnv *env, jobject thiz,
                                                                       jstring video_path) {
    if (!pRenderer) {
        LOGE("initDecoder, pRenderer is null.");
        return;
    }

    int videoWidth, videoHeight;
    const char *path = env->GetStringUTFChars(video_path, nullptr);
    //创建视频解码器
    pVideoDecoder = new VideoDecoder(path);
    pVideoDecoder->GetVideoSize(videoWidth, videoHeight);
    pRenderer->SetVideoSize(videoWidth, videoHeight);
    auto callback = std::bind(&VideoRenderer::VideoDecodeCallback, pRenderer, std::placeholders::_1);
    pVideoDecoder->Start(callback);
    env->ReleaseStringUTFChars(video_path, path);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_opengles_1android_examples_video_VideoView_shutdown(JNIEnv *env, jobject thiz) {
    if (pVideoDecoder) {
        pVideoDecoder->Shutdown();
    }

    if (pRenderer) {
        pRenderer->Shutdown();
    }
}