//
// Created by xulinkai on 2021/10/28.
//

#include "Texture.h"

Texture::Texture() {
    glGenTextures(1, &id);
}

Texture::~Texture() {
    glDeleteTextures(1, &id);
}

Texture *Texture::GenSingleChannelTexture(int width, int height, const void *pixels, int location) {
    auto* texture = new Texture();
    texture->location = location;
    texture->width = width;
    texture->height = height;
    texture->internalFormat = GL_LUMINANCE;
    texture->format = GL_LUMINANCE;
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D,0,texture->internalFormat,width, height,0,texture->format,
                 GL_UNSIGNED_BYTE, pixels);
    return texture;
}

Texture *Texture::GenRGBATexture(int width, int height, const void *pixels, int location) {
    auto* texture = new Texture();
    texture->location = location;
    texture->width = width;
    texture->height = height;
    texture->internalFormat = GL_RGBA;
    texture->format = GL_RGBA;
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D,0,texture->internalFormat,width, height,0,texture->format,
                 GL_UNSIGNED_BYTE, pixels);
    return texture;
}

void Texture::ActiveTexture() {
    glActiveTexture(GL_TEXTURE0 + location);
    glBindTexture(GL_TEXTURE_2D, id);

    //LOGD("Texture::ActiveTexture, id: %d, location: %d", id, location);
}

void Texture::UpdateData(uint8_t *pixels) {
    ActiveTexture();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, pixels);
}
