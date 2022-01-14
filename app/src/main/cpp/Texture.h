//
// Created by xulinkai on 2021/10/28.
//

#ifndef OPENGLES_ANDROID_TEXTURE_H
#define OPENGLES_ANDROID_TEXTURE_H
#include "Util.h"

class Texture {
private:
    char mPrintString[128];
public:
    GLuint id;
    GLint unit;
    GLint width;
    GLint height;
    GLint format;
    GLint internalFormat;

    Texture();
    ~Texture();
    static Texture* GenSingleChannelTexture(int width, int height, const void *pixels, int unit = 0);
    static Texture* GenRGBATexture(int width, int height, const void *pixels, int unit);
    void ActiveTexture();
    void UpdateData(uint8_t* pixels);

    const char* ToString() {
        sprintf(mPrintString, "{id: %d, unit: %d, width: %d, height: %d, format: %d}", id, unit, width, height, format);

        return mPrintString;
    }
};


#endif //OPENGLES_ANDROID_TEXTURE_H
