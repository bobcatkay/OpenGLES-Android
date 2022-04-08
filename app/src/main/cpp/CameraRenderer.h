//
// Created by xulinkai on 2021/10/24.
//

#ifndef OPENGLES_ANDROID_CAMERARENDERER_H
#define OPENGLES_ANDROID_CAMERARENDERER_H
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include <android/native_window_jni.h>
#include "Shader.h"
#include "Util.h"
#include "Texture.h"
#include <vector>

class CameraRenderer {
public:
    CameraRenderer(ANativeWindow* window, int surfaceWidth, int surfaceHeight);
    ~CameraRenderer() {};

    void OnDrawFrame(AHardwareBuffer* buffer, int width, int height);
    void OnSurfaceChanged(int width, int height, int rotation);
    void Release();
    void SetTextureStoreDir(const char* path);
    void BeginDump();

private:
    const GLuint VERTEX_COUNT = 6;

    Shader* pShader;
    ANativeWindow* pWindow;
    EGLSurface mEglSurface;
    EGLDisplay mDisplay;
    EGLContext mContext;
    int mWindowWidth = 0;
    int mWindowHeight = 0;
    GLuint mVAO;
    GLuint mTexId = 0;
    GLuint mLastBufferWidth = 0;
    GLuint mLastBufferHeight = 0;
    glm::mat4 mProjectionMatrix = glm::mat4(1.0f);
    glm::mat4 mTransformMatrix = glm::mat4(1.0f);
    GLint mRotation = 0;
    char *mStoreDir = new char[128];
    int mbFlag = 0;
    std::vector<Texture*> mTexColorBuffer;
    std::vector<GLuint> mFBO;
    const int DUMP_SIZE = 6;

    void UpdateTexture(AHardwareBuffer* buffer, int width, int height);
    void Init();
    void UpdateProjection();
    void UpdateTransform();
    void ReadTexture(const char* file, int width, int height);
};


#endif //OPENGLES_ANDROID_CAMERARENDERER_H
