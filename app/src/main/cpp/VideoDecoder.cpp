//
// Created by xulinkai on 2021/10/27.
//

#include "VideoDecoder.h"
#include <queue>

static std::mutex sDecodeMtx;
VideoDecoder::VideoDecoder(int fd)
{
    sprintf(mVideoPath, "/proc/self/fd/%d", fd);
    LOGD("VideoDecoder, %s", mVideoPath);

    int width = 0, height = 0;
    GetVideoSize(width, height);
    LOGD("VideoDecoder, width: %d, height: %d", width, height);
}

void VideoDecoder::Start(std::function<void(AVFrame* frame)> decodeCallback)
{
    std::thread t(&VideoDecoder::Decode,this,decodeCallback);
    t.detach();
}

int VideoDecoder::DecoderInit(AVCodec* codec, AVCodecContext*& vc, int vs)
{
    int code = 0;
    //Find vidoe decoder
    codec = avcodec_find_decoder(mFormatCtx->streams[vs]->codecpar->codec_id);
    if (!codec) {
        LOGE("DecoderInit, Failed to find video decoder! ");
        return -1;
    }

    //Init video decoder
    vc = avcodec_alloc_context3(codec);
    if (!vc) {
        LOGE("DecoderInit, avcodec_alloc_context3 failed.\n");
        return -2;
    }
    code = avcodec_parameters_to_context(vc, mFormatCtx->streams[vs]->codecpar);
    if (code < 0) {
        LOGE("DecoderInit, avcodec_parameters_to_context failed.\n");
        return -3;
    }

    //Open video decoder
    code = avcodec_open2(vc, NULL, NULL);
    return code;
}

void VideoDecoder::Decode(std::function<void(AVFrame* frame)> decodeCallback)
{
    AVCodec* vCodec = NULL;
    AVCodecContext* vc = NULL;

    int code = 0;
    int vs;
    /* open the input file */
    code = avformat_open_input(&mFormatCtx, mVideoPath, NULL, NULL);
    if (code) {
        LOGE("Decode, Cannot open input file '%s'\n", mVideoPath);
        return;
    }

    code = avformat_find_stream_info(mFormatCtx, NULL);
    if (code) {
        LOGE("Decode, Cannot find input stream information.\n");
        return;
    }

    /* find the video stream information */
    vs = av_find_best_stream(mFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &vCodec, 0);
    if (vs < 0) {
        LOGE("Decode, Cannot find a video stream in the input file\n");
        return;
    }

    AVCodecParameters* codecpar = mFormatCtx->streams[vs]->codecpar;
    mVideoWidth = codecpar->width;
    mVideoHeight = codecpar->height;

    int fps = round(R2d(mFormatCtx->streams[vs]->avg_frame_rate));
    LOGD("Decode, FPS: %d.", fps);

    code = DecoderInit(vCodec, vc, vs);

    if (code) {
        LOGE("Decode, Failed to open video decoder! %s ", av_err2str(code));
        return;
    }


    //Read frames
    AVPacket* pkt = av_packet_alloc();
    double tb = R2d(mFormatCtx->streams[vs]->time_base);

    std::thread t(&VideoDecoder::SendFrame, this, decodeCallback, tb);
    t.detach();


    //解码帧最大缓存数量
    int maxCacheFrames = 5;

    AVFrame* frame = av_frame_alloc();
    AVFrame* swFrame = av_frame_alloc();
    AVFrame* tempFrame = NULL;
    uint8_t* buffer = NULL;
    int size;
    for (;;) {
        if (!bRun) {
            LOGE("Decode, Decoder Exit.\n");
            break;
        }
        while (bPause)
        {
            Sleep(100);
        }

        code = av_read_frame(mFormatCtx, pkt);
        if (code) {
            LOGD("Decode, Frame read finish.\n");

            av_packet_unref(pkt);
            int pos = 0;
            av_seek_frame(mFormatCtx, vs, pos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
            continue;
        }
        if (pkt->stream_index != vs) {
            av_packet_unref(pkt);
            continue;
        }


        code = avcodec_send_packet(vc, pkt);
        if (code) {
            LOGE("Decode, Error during decoding\n");
            break;
        }
        //clear
        av_packet_unref(pkt);

//        if (code) {
//            LOGE("avcodec_send_packet failed!\n");
//            continue;
//        }

        for (;;) {
            if (!bRun) {
                break;
            }
            code = avcodec_receive_frame(vc, frame);
            if (code) {
                //LOGD("avcodec_receive_frame failed!\n");
                break;
            }

            tempFrame = frame;
            AVFrame* f = av_frame_clone(tempFrame);
            av_frame_unref(tempFrame);

            sDecodeMtx.lock();
            mCachedFrames.push_back(f);
            mCacheSize ++;
            sDecodeMtx.unlock();

            while (bRun && mCacheSize >= maxCacheFrames) {
                Sleep(2);
            }
        }


    }
    av_frame_free(&frame);
    av_frame_free(&swFrame);

    av_packet_free(&pkt);
    avcodec_free_context(&vc);
    av_buffer_unref(&mHwDeviceCtx);
    Clear();
    FreeCacheFrames();
}

void VideoDecoder::SendFrame(std::function<void(AVFrame* frame)> decodeCallback,double tb)
{
    long long refTime = GetNowMs();
    AVFrame* frame = NULL;
    int frameCount = 0;
    long long frameCountStart = GetNowMs();
    long long pauseStart = 0;
    for (;;) {
        if (!bRun) {
            LOGE("SendFrame, Send data stop.\n");
            break;
        }
        while (bPause)
        {
            if (!pauseStart) {
                pauseStart = GetNowMs();
            }
            Sleep(100);
        }

        if (pauseStart) {
            refTime += GetNowMs() - pauseStart;
            pauseStart = 0;
        }

        if (!frame) {
            //LOGD("cacheSize:%d\n",mCacheSize);
            sDecodeMtx.lock();
            if (mCacheSize == 0) {
                sDecodeMtx.unlock();
                Sleep(5);
                continue;
            }
            //std::lock_guard<std::mutex> decodeLock(sDecodeMtx);
            frame = mCachedFrames.front();
            mCachedFrames.pop_front();
            mCacheSize--;
            sDecodeMtx.unlock();
            //LOGD("Fetch frame.\n");
        }

        double pts = tb * frame->pts * 1000;
        if (pts < mCurPts) {
            refTime = GetNowMs() - pts;
            LOGD("Replay. Now:%ld refTime:%lld pts:%.1f\n", GetNowMs(), refTime, pts);
        }
        mCurPts = pts;
        for (;;) {
            if (GetNowMs() - refTime >= mCurPts) {
                //double t = GetGLTime();
                decodeCallback(frame);
                av_frame_free(&frame);

                //LOGD("Send frame time:%.3f\n", GetGLTime() - t);

                //DEBUG
                frameCount++;
                if (GetNowMs() - frameCountStart >= 1000) {
                    LOGD("Play fps is %d\n", frameCount);
                    frameCountStart = GetNowMs();
                    frameCount = 0;
                }
                break;
            }
            Sleep(2);
        }
    }
    av_frame_free(&frame);

}

void VideoDecoder::GetVideoSize(int& width, int& height)
{
    int vs = FindVideoStream();
    if (vs < 0) {
        LOGE("GetVideoSize, Find video stream failed!\n");
        return;
    }
    AVCodecParameters* codecpar = mFormatCtx->streams[vs]->codecpar;
    width = codecpar->width;
    height = codecpar->height;
    Clear();
}

int VideoDecoder::FindVideoStream()
{
    //std::string fileName = std::string(mVideoPath);
    AVInputFormat* fmt = NULL;
    AVDictionary* dict = NULL;
    av_register_all();
    avcodec_register_all();
    avformat_network_init();
    int code = avformat_open_input(&mFormatCtx, mVideoPath, fmt, &dict);
    if (code) {
        LOGE("FindVideoStream, avformat_open_input failed:%s\n", av_err2str(code));
        return -1;
    }
    code = avformat_find_stream_info(mFormatCtx, NULL);
    if (code) {
        LOGE("FindVideoStream, avformat_find_stream_info failed!\n");
    }
    LOGD("FindVideoStream, duration = %lld nb_streams = %d\n", mFormatCtx->duration, mFormatCtx->nb_streams);

    int vs = -1;
    for (int i = 0; i < mFormatCtx->nb_streams; i++) {
        AVStream* avs = mFormatCtx->streams[i];
        if (avs->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            vs = i;
            break;
        }
    }
    if (vs < 0) {
        LOGE("FindVideoStream, find video stream failed!\n");
        return -1;
    }
    return vs;
}

void VideoDecoder::Shutdown()
{
    bPause = false;
    bRun = false;
}

void VideoDecoder::Pause()
{
    bPause = !bPause;
}

void VideoDecoder::Clear()
{
    avformat_close_input(&mFormatCtx);
}

void VideoDecoder::FreeCacheFrames()
{
    LOGD("FreeCacheFrames");
    std::lock_guard<std::mutex> decodeLock(sDecodeMtx);
    for (auto& f: mCachedFrames) {
        av_frame_free(&f);
    }
    mCacheSize = 0;
    mCachedFrames.clear();
    LOGD("FreeCacheFrames, Exit.");
}