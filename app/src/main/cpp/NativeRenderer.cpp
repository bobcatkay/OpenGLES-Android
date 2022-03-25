#define STB_IMAGE_IMPLEMENTATION

#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/native_window_jni.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <unistd.h>
#include "Texture.h"
#include "stb_image.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/ext.hpp"
#include <thread>

void InitContext(JNIEnv *env, jobject surface);
GLuint InitShaderProgram(const char* vertexCode, const char* fragCode);
GLuint LoadShader(const char *code, GLenum type);
void OnDraw();
void InitVertex();
Texture* InitTexture(const char*);

ANativeWindow *pWindow = nullptr;
bool mbShutDown = false;
GLuint mShaderProgram;
EGLSurface mEglSurface;
EGLDisplay mDisplay;
EGLContext mContext;
int mWindowWidth = 0;
int mWindowHeight = 0;
GLuint mVAO;
GLuint mVertexCount = 3;
const int TEXTURE_UNIT = 1;
glm::mat4 mProjectionMatrix = glm::mat4(1.0f);
glm::mat4 mTransformMatrix = glm::mat4(1.0f);

void InitOffScreenContext() {
    // EGL config attributes
    const EGLint confAttr[] =
            {
                    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,// very important!
                    EGL_SURFACE_TYPE,EGL_PBUFFER_BIT,//EGL_WINDOW_BIT EGL_PBUFFER_BIT we will create a pixelbuffer surface
                    EGL_RED_SIZE,   8,
                    EGL_GREEN_SIZE, 8,
                    EGL_BLUE_SIZE,  8,
                    EGL_ALPHA_SIZE, 8,// if you need the alpha channel
                    EGL_DEPTH_SIZE, 8,// if you need the depth buffer
                    EGL_STENCIL_SIZE,8,
                    EGL_NONE
            };
    // EGL context attributes
    const EGLint ctxAttr[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,// very important!
            EGL_NONE
    };
    // surface attributes
    // the surface size is set to the input frame size
    const EGLint surfaceAttr[] = {
            EGL_WIDTH,512,
            EGL_HEIGHT,512,
            EGL_NONE
    };
    EGLint eglMajVers, eglMinVers;
    EGLint numConfigs;

    EGLDisplay eglDisp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(eglDisp == EGL_NO_DISPLAY)
    {
        //Unable to open connection to local windowing system
        LOGD("Unable to open connection to local windowing system");
    }
    if(!eglInitialize(eglDisp, &eglMajVers, &eglMinVers))
    {
        // Unable to initialize EGL. Handle and recover
        LOGD("Unable to initialize EGL");
    }
    LOGD("EGL init with version %d.%d", eglMajVers, eglMinVers);
    // choose the first config, i.e. best config
    EGLConfig eglConf;
    if(!eglChooseConfig(eglDisp, confAttr, &eglConf, 1, &numConfigs))
    {
        LOGD("some config is wrong");
    }
    else
    {
        LOGD("all configs is OK");
    }
    // create a pixelbuffer surface
    EGLSurface eglSurface = eglCreatePbufferSurface(eglDisp, eglConf, surfaceAttr);
    if(eglSurface == EGL_NO_SURFACE)
    {
        switch(eglGetError())
        {
            case EGL_BAD_ALLOC:
                // Not enough resources available. Handle and recover
                LOGD("Not enough resources available");
                break;
            case EGL_BAD_CONFIG:
                // Verify that provided EGLConfig is valid
                LOGD("provided EGLConfig is invalid");
                break;
            case EGL_BAD_PARAMETER:
                // Verify that the EGL_WIDTH and EGL_HEIGHT are
                // non-negative values
                LOGD("provided EGL_WIDTH and EGL_HEIGHT is invalid");
                break;
            case EGL_BAD_MATCH:
                // Check window and EGLConfig attributes to determine
                // compatibility and pbuffer-texture parameters
                LOGD("Check window and EGLConfig attributes");
                break;
        }
    }
    EGLContext eglCtx = eglCreateContext(eglDisp, eglConf, mContext, ctxAttr);
    if(eglCtx == EGL_NO_CONTEXT)
    {
        EGLint error = eglGetError();
        if(error == EGL_BAD_CONFIG)
        {
            // Handle error and recover
            LOGD("EGL_BAD_CONFIG");
        }
    }
    if(!eglMakeCurrent(eglDisp, eglSurface, eglSurface, eglCtx))
    {
        LOGD("MakeCurrent failed");
    }
    LOGD("initialize success!");
}

void PrepareTexture(const char* path, Texture* texture) {
    InitOffScreenContext();
    Texture *pTexture = InitTexture(path);
    texture->id = pTexture->id;
    texture->unit = pTexture->unit;
    texture->width = pTexture->width;
    texture->height = pTexture->height;
    texture->format = pTexture->format;
}


void InitMatrix(int texWidth, int texHeight) {
    mProjectionMatrix = glm::mat4(1.0f);
    float aspectRatio = mWindowWidth > mWindowHeight ?
                        (float) mWindowWidth / (float) mWindowHeight :
                        (float) mWindowHeight / (float) mWindowWidth;

    if (mWindowWidth > mWindowHeight) {
        mProjectionMatrix = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f);
    } else {
        mProjectionMatrix = glm::ortho(-1.0f, 1.0f, -aspectRatio, aspectRatio, -1.0f, 1.0f);
    }

    float scaleY = 1.0f;
    float scaleX = 1.0f;

    float texRatio = (float) texHeight / texWidth;
    float targetHeight = mWindowWidth * texRatio;

    if (targetHeight <= mWindowHeight) {
        scaleX = (mWindowWidth <= mWindowHeight) ? 1.0f : ((float) mWindowWidth / mWindowHeight);
        scaleY = (mWindowWidth <= mWindowHeight) ? (targetHeight / mWindowWidth) : (targetHeight / mWindowHeight);
    } else {
        float targetWidth = (float) mWindowHeight / texRatio;
        scaleX = (mWindowWidth <= mWindowHeight) ? (targetWidth / mWindowWidth) : (targetWidth / mWindowHeight);
        scaleY = (mWindowWidth <= mWindowHeight) ? ((float) mWindowHeight / mWindowWidth) : 1.0f;
    }

    mTransformMatrix = glm::mat4(1.0f);
    mTransformMatrix = glm::scale(mTransformMatrix, glm::vec3(scaleX, scaleY, 1.0f));
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_opengles_1android_examples_native_1render_NativeRenderView_init(JNIEnv *env,
                                                                                jobject thiz,
                                                                                jobject surface,
                                                                                jstring vertex_code,
                                                                                jstring frag_code,
                                                                                jstring file_path) {
    // Init EGL context
    InitContext(env, surface);

    const char *filePath = env->GetStringUTFChars(file_path, nullptr);
    Texture pTexture;
    std::thread tTex(PrepareTexture, filePath, &pTexture);
    tTex.join();
    env->ReleaseStringUTFChars(file_path, filePath);

    InitVertex();

    LOGD("init, pTexture: %s.", pTexture.ToString());

    InitMatrix(pTexture.width, pTexture.height);

    // Init shader mShaderProgram
    const char *vertexCode = env->GetStringUTFChars(vertex_code, nullptr);
    const char *fragCode = env->GetStringUTFChars(frag_code, nullptr);
    mShaderProgram = InitShaderProgram(vertexCode, fragCode);
    env->ReleaseStringUTFChars(vertex_code, vertexCode);
    env->ReleaseStringUTFChars(frag_code, fragCode);

    if (!mShaderProgram) {
        return;
    }

    glUseProgram(mShaderProgram);
    glUniform1i(glGetUniformLocation(mShaderProgram, "uTexture"), pTexture.unit);
    pTexture.ActiveTexture();
    glUniformMatrix4fv(glGetUniformLocation(mShaderProgram, "uTransform"), 1, GL_FALSE, glm::value_ptr(mTransformMatrix));
    glUniformMatrix4fv(glGetUniformLocation(mShaderProgram, "uProjection"), 1, GL_FALSE, glm::value_ptr(mProjectionMatrix));

    while (!mbShutDown) {
        glViewport(0, 0, mWindowWidth, mWindowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(1.0f,1.0f,1.0f,1.0f);

        OnDraw();

        eglSwapBuffers(mDisplay, mEglSurface);
    }

    glDeleteBuffers(1, &mVAO);
    glDeleteProgram(mShaderProgram);
    eglDestroySurface(mDisplay, mEglSurface);
    eglDestroyContext(mDisplay, mContext);
    ANativeWindow_release(pWindow);
    mbShutDown = false;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_opengles_1android_examples_native_1render_NativeRenderView_stop(JNIEnv *env,
                                                                                jobject thiz) {
    mbShutDown = true;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_github_opengles_1android_examples_native_1render_NativeRenderView_setSurfaceSize(
        JNIEnv *env, jobject thiz, jint width, jint height) {
    mWindowWidth = width;
    mWindowHeight = height;
}

void InitContext(JNIEnv *env, jobject surface) {
    EGLint majorVersion;
    EGLint minorVersion;
    pWindow = ANativeWindow_fromSurface(env, surface);

    // Create and initialize EGLDisplay
    mDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    if (EGL_NO_DISPLAY == mDisplay) {
        LOGE("InitContext, eglGetDisplay failed!");
        return;
    }

    if (!eglInitialize(mDisplay, &majorVersion, &minorVersion)) {
        LOGE("InitContext, eglInitialize failed!");
        return;
    }

    // Config and create EGLSurface
    EGLConfig config;
    EGLint configNum;
    EGLint configAttribs[] = {
            EGL_RENDERABLE_TYPE, EGL_WINDOW_BIT,
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE
    };

    if (!eglChooseConfig(mDisplay, configAttribs, &config, 1, &configNum)) {
        EGLint error = eglGetError();
        LOGE("InitContext, eglChooseConfig failed! error: %d.", error);
        return;
    }

    mEglSurface = eglCreateWindowSurface(mDisplay, config, pWindow, nullptr);

    if (EGL_NO_SURFACE == mEglSurface) {
        EGLint error = eglGetError();
        LOGE("InitContext, eglCreateWindowSurface failed! error: %d.", error);
        return;
    }

    const EGLint ctxAttr[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    mContext = eglCreateContext(mDisplay, config, EGL_NO_CONTEXT, ctxAttr);

    if (EGL_NO_CONTEXT == mContext) {
        EGLint error = eglGetError();
        LOGE("InitContext, eglCreateContext failed! error: %d.", error);
        return;
    }

    if (!eglMakeCurrent(mDisplay, mEglSurface, mEglSurface, mContext)) {
        EGLint error = eglGetError();
        LOGE("InitContext, eglMakeCurrent failed! error: %d.", error);
        return;
    }
}

GLuint InitShaderProgram(const char* vertexCode, const char* fragCode) {
    GLuint program = glCreateProgram();

    if (0 == program) {
        LOGE("InitShaderProgram, create mShaderProgram failed!");
        return 0;
    }

    GLint vertexShader = LoadShader(vertexCode, GL_VERTEX_SHADER);
    GLint fragShader = LoadShader(fragCode, GL_FRAGMENT_SHADER);

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragShader);

    glLinkProgram(program);

    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);

    if (!linked) {
        GLchar eLog[1024] = { 0 };
        glGetProgramInfoLog(program, sizeof(eLog), NULL, eLog);
        LOGE("InitShaderProgram, Error linking mShaderProgram: '%s'", eLog);
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

GLuint LoadShader(const char *code, GLenum type) {
    LOGD("LoadShader");

    // Create the shader object
    GLuint shader = glCreateShader(type);

    if (0 == shader) {
        LOGE("glCreateShader %d failed!", type);
        return 0;
    }

    // Load the shader source
    glShaderSource(shader, 1, &code, nullptr);

    // Compile the shader
    glCompileShader(shader);

    // Check the compile status
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (!status) {
        GLchar eLog[1024] = { 0 };
        glGetShaderInfoLog(shader, sizeof(eLog), NULL, eLog);
        LOGE("LoadShader, Error compiling the %d shader: '%s'", type, eLog);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

void InitVertex() {
    float vertices[] = {
            // positions            //texture
            -1.0f,  1.0f, 0.0f,    0.0f, 0.0f,  // top left
            -1.0f, -1.0f, 0.0f,    0.0f, 1.0f, // bottom left
            1.0f,  1.0f, 0.0f,     1.0f, 0.0f, // top right
            1.0f, -1.0f, 0.0f,     1.0f, 1.0f, // bottom right
    };

    mVertexCount = 4;
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float ), (void*) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, true, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}

Texture* InitTexture(const char* path) {
    int width, height, channels;
    unsigned char* data = stbi_load(path, &width, &height, &channels, 0);

    if (!data) {
        LOGE("InitTexture, Failed to load image at %s.", path);

        return nullptr;
    }

    Texture *texture = Texture::GenTexture(width, height, data, TEXTURE_UNIT);
    LOGD("InitTexture, texture: %s, channels: %d.", texture->ToString(), channels);
    
    return texture;
}

void OnDraw() {
    glUseProgram(mShaderProgram);
    glBindVertexArray(mVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, mVertexCount);
}