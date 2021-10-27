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
#include <list>
#include <vector>
#include <chrono>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#include <libavutil/opt.h>
}

class VideoDecoder
{
public:
    VideoDecoder() {};
    VideoDecoder(int fd);
    void Start(std::function<void(AVFrame*)> decodeCallback);
    void Decode(std::function<void(AVFrame*)> decodeCallback);
    void SendFrame(std::function<void(AVFrame*)> decodeCallback,double);
    void GetVideoSize(int& width, int& height);
    int FindVideoStream();
    void Pause();
    void Shutdown();
    void Clear();
private:
    AVFormatContext* mFormatCtx = NULL;
    char mVideoPath[128];
    int mVideoWidth, mVideoHeight;
    double mCurPts = 0;
    bool bRun = true;
    bool bPause = false;

    std::list<AVFrame*> mCachedFrames;
    std::vector<AVFrame*> mFramesQue;
    int mCacheSize = 0;
    bool bReplay = false;
    struct timespec currentTime;

    AVBufferRef* mHwDeviceCtx = NULL;

    int DecoderInit(AVCodec* codec, AVCodecContext*& vc, int vs);
    void FreeCacheFrames();

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
