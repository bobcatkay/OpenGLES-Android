//
// Created by xulinkai on 2021/10/27.
//

#include "VideoDecoder.h"
#include <queue>

static std::mutex sDecodeMtx;

VideoDecoder::VideoDecoder(const char* path) {
    strcpy(mVideoPath, path);
    LOGD("VideoDecoder, %s", mVideoPath);
}

void VideoDecoder::Start(std::function<void(AVFrame* frame)> decodeCallback) {
    std::thread t(&VideoDecoder::Decode, this, decodeCallback);
    t.detach();
}

int VideoDecoder::DecoderInit(AVCodec* codec, AVCodecContext*& vc, int vs) {
    int code = 0;
    //Find vidoe decoder
    codec = avcodec_find_decoder(pFormatCtx->streams[vs]->codecpar->codec_id);
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
    code = avcodec_parameters_to_context(vc, pFormatCtx->streams[vs]->codecpar);
    if (code < 0) {
        LOGE("DecoderInit, avcodec_parameters_to_context failed.\n");
        return -3;
    }

    //Open video decoder
    code = avcodec_open2(vc, NULL, NULL);
    return code;
}

void VideoDecoder::Decode(std::function<void(AVFrame* frame)> decodeCallback) {
    AVCodec* vCodec = nullptr;

    int vs;
    /* open the input file */
    int code = avformat_open_input(&pFormatCtx, mVideoPath, NULL, NULL);
    if (code) {
        LOGE("Decode, Cannot open input file '%s'\n", mVideoPath);
        return;
    }

    code = avformat_find_stream_info(pFormatCtx, NULL);
    if (code) {
        LOGE("Decode, Cannot find input stream information.\n");
        return;
    }

    /* find the video stream information */
    vs = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &vCodec, 0);
    if (vs < 0) {
        LOGE("Decode, Cannot find a video stream in the input file\n");
        return;
    }

    int fps = round(R2d(pFormatCtx->streams[vs]->avg_frame_rate));
    LOGD("Decode, FPS: %d.", fps);

    code = DecoderInit(vCodec, pCodecContext, vs);

    if (code) {
        LOGE("Decode, Failed to open video decoder! %s ", av_err2str(code));
        return;
    }

    //Read frames
    AVPacket* pkt = av_packet_alloc();
    double tb = R2d(pFormatCtx->streams[vs]->time_base);

    std::thread t(&VideoDecoder::SendFrame, this, decodeCallback, tb);
    t.detach();


    //解码帧最大缓存数量
    int maxCacheFrames = 5;

    AVFrame* frame = av_frame_alloc();
    AVFrame* tempFrame = NULL;

    for (;;) {
        if (!bRun) {
            LOGE("Decode, Decoder Exit.\n");
            break;
        }
        while (bPause)
        {
            Sleep(100);
        }

        code = av_read_frame(pFormatCtx, pkt);
        if (code) {
            LOGD("Decode, Frame read finish.\n");

            av_packet_unref(pkt);
            int pos = 0;
            av_seek_frame(pFormatCtx, vs, pos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
            continue;
        }
        if (pkt->stream_index != vs) {
            av_packet_unref(pkt);
            continue;
        }


        code = avcodec_send_packet(pCodecContext, pkt);
        if (code) {
            LOGE("Decode, Error during decoding\n");
            break;
        }
        //clear
        av_packet_unref(pkt);

        for (;;) {
            if (!bRun) {
                break;
            }
            code = avcodec_receive_frame(pCodecContext, frame);
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
                Sleep(1);
            }
        }


    }

    av_frame_free(&frame);
    av_packet_free(&pkt);
    FreeCacheFrames();
    avformat_close_input(&pFormatCtx);
    avcodec_free_context(&pCodecContext);
}

void VideoDecoder::SendFrame(std::function<void(AVFrame* frame)> decodeCallback, double tb) {
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
            //LOGD("SendFrame, cacheSize:%d\n",mCacheSize);
            sDecodeMtx.lock();
            if (mCacheSize == 0) {
                sDecodeMtx.unlock();
                Sleep(5);
                continue;
            }
            frame = mCachedFrames.front();
            mCachedFrames.pop_front();
            mCacheSize--;
            sDecodeMtx.unlock();
            //LOGD("Fetch frame.\n");
        }

        double pts = tb * frame->pts * 1000;
        if (pts < mCurPts) {
            refTime = GetNowMs() - pts;
            LOGD("SendFrame, Replay. Now:%ld refTime:%lld pts:%.1f\n", GetNowMs(), refTime, pts);
        }
        mCurPts = pts;
        for (;;) {
            if (GetNowMs() - refTime >= mCurPts) {
                decodeCallback(frame);
                av_frame_free(&frame);

                //DEBUG
                frameCount++;
                if (GetNowMs() - frameCountStart >= 1000) {
                    LOGD("SendFrame, Play fps is %d\n", frameCount);
                    frameCountStart = GetNowMs();
                    frameCount = 0;
                }
                break;
            }
            Sleep(1);
        }
    }
    av_frame_free(&frame);
}

void VideoDecoder::GetVideoSize(int& width, int& height) {
    int vs = FindVideoStream();
    if (vs < 0) {
        LOGE("GetVideoSize, Find video stream failed!\n");
        return;
    }
    AVCodecParameters* codecpar = pFormatCtx->streams[vs]->codecpar;
    width = codecpar->width;
    height = codecpar->height;
    avformat_close_input(&pFormatCtx);
}

int VideoDecoder::FindVideoStream() {
    AVInputFormat* fmt = NULL;
    AVDictionary* dict = NULL;
    av_register_all();
    avcodec_register_all();
    int code = avformat_open_input(&pFormatCtx, mVideoPath, fmt, &dict);
    if (code) {
        LOGE("FindVideoStream, avformat_open_input failed:%s\n", av_err2str(code));
        return -1;
    }
    code = avformat_find_stream_info(pFormatCtx, NULL);
    if (code) {
        LOGE("FindVideoStream, avformat_find_stream_info failed!\n");
    }
    LOGD("FindVideoStream, duration = %lld nb_streams = %d\n", pFormatCtx->duration, pFormatCtx->nb_streams);

    int vs = -1;
    for (int i = 0; i < pFormatCtx->nb_streams; i++) {
        AVStream* avs = pFormatCtx->streams[i];
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

void VideoDecoder::Shutdown() {
    bPause = false;
    bRun = false;
}

void VideoDecoder::Pause() {
    bPause = !bPause;
}

void VideoDecoder::FreeCacheFrames() {
    LOGD("FreeCacheFrames");
    sDecodeMtx.lock();
    for (auto& f: mCachedFrames) {
        av_frame_free(&f);
    }
    mCacheSize = 0;
    mCachedFrames.clear();
    sDecodeMtx.unlock();
    LOGD("FreeCacheFrames, Exit.");
}
