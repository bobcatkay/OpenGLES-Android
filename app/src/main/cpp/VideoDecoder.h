//
// Created by xulinkai on 2021/10/27.
//
#ifndef OPENGLES_ANDROID_VIDEODECODER_H
#define OPENGLES_ANDROID_VIDEODECODER_H

#include <iostream>
#include <thread>
#include <functional>
#include "Util.h"
#include <mutex>
#include <chrono>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

class VideoDecoder
{
public:
    VideoDecoder() {};
    VideoDecoder(int fd);
    void Start(std::function<void(AVFrame*)> decodeCallback);
    void Decode(std::function<void(AVFrame*)> decodeCallback);
    void Shutdown();
private:
    AVFormatContext* mFormatCtx = nullptr;
    AVCodecContext* pCodecContext = nullptr;

    char mVideoPath[128];

    bool bRun = true;
    struct timespec currentTime;

    int DecoderInit(AVCodec* codec, AVCodecContext*& vc, int vs);

    double R2d(AVRational r) {
        return r.num == 0 || r.den == 0 ? 0 : (double)r.num / (double)r.den;
    }

    void Sleep(long long time) {
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
    }

    long GetNowMs() {
        clock_gettime(CLOCK_REALTIME, &currentTime);
        return currentTime.tv_sec * 1000 + currentTime.tv_nsec / 1000000;
    }
};


#endif //OPENGLES_ANDROID_VIDEODECODER_H
