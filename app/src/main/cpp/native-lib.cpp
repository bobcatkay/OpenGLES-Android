#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/native_window_jni.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <unistd.h>

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,"native-lib",__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"native-lib",__VA_ARGS__)

void InitContext(JNIEnv *env, jobject surface);
GLuint InitShaderProgram(const char* vertexCode, const char* fragCode);
GLuint LoadShader(const char *code, GLenum type);
void onDraw();
void initVertex();

bool mbShutDown = false;
GLuint mShaderProgram;
EGLSurface mEglSurface;
EGLDisplay mDisplay;
EGLContext mContext;
int mWindowWidth = 0;
int mWindowHeight = 0;
GLuint mVAO;
GLuint mVertexCount = 3;

extern "C"
JNIEXPORT void JNICALL
Java_com_github_opengles_1android_examples_native_1render_NativeRenderView_init(JNIEnv *env,
                                                                                jobject thiz,
                                                                                jobject surface,
                                                                                jstring vertex_code,
                                                                                jstring frag_code) {
    // Init EGL context
    InitContext(env, surface);

    // Init shader mShaderProgram
    const char *vertexCode = env->GetStringUTFChars(vertex_code, nullptr);
    const char *fragCode = env->GetStringUTFChars(frag_code, nullptr);
    mShaderProgram = InitShaderProgram(vertexCode, fragCode);
    env->ReleaseStringUTFChars(vertex_code, vertexCode);
    env->ReleaseStringUTFChars(frag_code, fragCode);

    if (!mShaderProgram) {
        return;
    }

    initVertex();

    while (!mbShutDown) {
        glViewport(0, 0, mWindowWidth, mWindowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(1.0f,1.0f,1.0f,1.0f);

        onDraw();

        eglSwapBuffers(mDisplay, mEglSurface);
    }

    glDeleteBuffers(1, &mVAO);
    glDeleteProgram(mShaderProgram);
    eglDestroySurface(mDisplay, mEglSurface);
    eglDestroyContext(mDisplay, mContext);
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
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);

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

    mEglSurface = eglCreateWindowSurface(mDisplay, config, window, nullptr);

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

void initVertex() {
    float vertices[] = {
            // positions       //color
            0,     0.5f, 0,   1.0f, 0, 0,
            -1.0f, -0.5f, 0,   0, 1.0f, 0,
            1.0f, -0.5f, 0,    0, 0, 1.0f,
    };

    mVertexCount = 3;
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float ), (void*) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, true, 6 * sizeof(float), (void*)(3 * sizeof(float)));
}

void onDraw() {
    glUseProgram(mShaderProgram);
    glBindVertexArray(mVAO);
    glDrawArrays(GL_TRIANGLES, 0, mVertexCount);
}