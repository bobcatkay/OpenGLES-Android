//
// Created by xulinkai on 2021/10/24.
//
#include "Util.h"

static AAssetManager* pAssetManager;

void JniInit(JNIEnv* env, jobject assetManager) {
    pAssetManager = AAssetManager_fromJava(env, assetManager);
}

std::string ReadFileFromAssets(const char* fileName) {
    LOGD("ReadFileFromAssets, file: %s.", fileName);

    std::string content;

    if (nullptr == pAssetManager) {
        LOGE("ReadFileFromAssets, %s","AAssetManager==NULL");
        return nullptr;
    }

    /*获取文件名并打开*/
    AAsset* asset = AAssetManager_open(pAssetManager, fileName,AASSET_MODE_UNKNOWN);

    if (nullptr == asset) {
        LOGE("ReadFileFromAssets, %s","asset==NULL");
        return nullptr;
    }

    /*获取文件大小*/
    off_t bufferSize = AAsset_getLength(asset);
    char *buffer = (char *) malloc(bufferSize+1);
    buffer[bufferSize] = 0;
    int numBytesRead = AAsset_read(asset, buffer, bufferSize);

    if (numBytesRead > 0) {
        content = std::string(buffer);
    }

    LOGD("ReadFileFromAssets, %s",buffer);
    free(buffer);
    /*关闭文件*/
    AAsset_close(asset);

    return content;
}
