package com.github.opengles_android.examples.triangle;

import android.content.Context;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.util.Size;

import com.github.opengles_android.common.Utils;

import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import static android.opengl.GLES30.*;
import static com.github.opengles_android.common.Utils.FLOAT_SIZE;

public class TriangleRender implements GLSurfaceView.Renderer {
    private final static String TAG = "TriangleRender";

    private int[] mVAO;
    private Context mContext;
    private int mScreenWidth;
    private int mScreenHeight;
    private int mVertexCount;
    private int mShaderProgram;
    long mStartTime;

    public TriangleRender(Context context) {
        mContext = context;
        Size screenSize = Utils.getScreenResolution(context);
        mScreenWidth = screenSize.getWidth();
        mScreenHeight = screenSize.getHeight();
    }

    @Override
    public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {
        initVertex();
        initShader("triangle.vert", "triangle.frag");
        mStartTime = System.currentTimeMillis();
    }

    @Override
    public void onSurfaceChanged(GL10 gl10, int i, int i1) {

    }

    @Override
    public void onDrawFrame(GL10 gl10) {
        glViewport(0, 0, mScreenWidth, mScreenHeight);

        glClearColor(1f, 1f, 1f, 0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(mShaderProgram);
        float elapseTime = (System.currentTimeMillis() - mStartTime) / 1000.0f;
        glUniform1f(glGetUniformLocation(mShaderProgram, "time"), elapseTime);
        glBindVertexArray(mVAO[0]);
        glDrawArrays(GL_TRIANGLES, 0, mVertexCount);
    }

    private void initVertex() {
        Log.d(TAG, "InitVertex");

        float vertices[] = {
                // positions       //color
                0f,     0.5f, 0f,   1.0f, 0f, 0f,
                -1.0f, -0.5f, 0f,   0f, 1.0f, 0f,
                1.0f, -0.5f, 0f,    0f, 0f, 1.0f,
        };

        FloatBuffer vertexBuffer = Utils.floatArrayToBuffer(vertices);
        mVertexCount = 3;

        mVAO = new int[1];
        int[] vbo = new int[1];
        glGenVertexArrays(1, mVAO, 0);
        glBindVertexArray(mVAO[0]);

        glGenBuffers(1, vbo, 0);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        int size = vertexBuffer.capacity() * FLOAT_SIZE;
        glBufferData(GL_ARRAY_BUFFER, size, vertexBuffer, GLES20.GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, true, 6 * FLOAT_SIZE, 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, true, 6 * FLOAT_SIZE, 3 * FLOAT_SIZE);
    }

    private void initShader(String vertexFileName, String fragmentFileName) {
        String vertexCode = Utils.getStringFromAssets(mContext, vertexFileName);
        String fragCode = Utils.getStringFromAssets(mContext, fragmentFileName);
        int vertexShader = loadShader(vertexCode, GL_VERTEX_SHADER);
        int fragShader = loadShader(fragCode, GL_FRAGMENT_SHADER);

        mShaderProgram = glCreateProgram();

        if (0 == mShaderProgram) {
            Log.e(TAG, "createProgram: Create mShaderProgram error!", new Throwable());

            return;
        }

        glAttachShader(mShaderProgram, vertexShader);
        glAttachShader(mShaderProgram, fragShader);

        glLinkProgram(mShaderProgram);

        int[] result = new int[1];
        glGetProgramiv(mShaderProgram, GL_LINK_STATUS, result, 0);

        if (0 == result[0]) {
            String log = glGetProgramInfoLog(mShaderProgram);
            Log.e(TAG, "loadShader: Link mShaderProgram error!" + log, new Throwable());
        }
    }

    private int loadShader(String code, int type) {
        int shader = glCreateShader(type);
        glShaderSource(shader, code);
        glCompileShader(shader);

        int[] result = new int[1];
        glGetShaderiv(shader, GL_COMPILE_STATUS, result, 0);

        if (0 == result[0]) {
            String log = glGetShaderInfoLog(shader);
            Log.e(TAG, "loadShader: Compile shader error!\n"
                    + "code: " + code + "\n"
                    + log, new Throwable());
            return 0;
        }

        return shader;
    }
}
