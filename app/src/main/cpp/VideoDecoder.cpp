//
// Created by xulinkai on 2021/10/27.
//

#include "VideoDecoder.h"

VideoDecoder::VideoDecoder(int fd) {

}

void VideoDecoder::Start(std::function<void(AVFrame *)> decodeCallback) {

}

void VideoDecoder::Decode(std::function<void(AVFrame *)> decodeCallback) {

}

void VideoDecoder::Shutdown() {

}

int VideoDecoder::DecoderInit(AVCodec *codec, AVCodecContext *&vc, int vs) {
    return 0;
}
