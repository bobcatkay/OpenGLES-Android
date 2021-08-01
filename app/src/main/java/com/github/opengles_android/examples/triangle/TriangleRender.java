package com.github.opengles_android.examples.triangle;

import android.content.Context;
import android.opengl.GLES20;
import android.opengl.GLES30;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.util.Size;

import com.github.opengles_android.common.Shader;
import com.github.opengles_android.common.Utils;

import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import static com.github.opengles_android.common.Utils.FLOAT_SIZE;

public class TriangleRender implements GLSurfaceView.Renderer {
    private final static String TAG = "TriangleRender";

    private int[] mVAO;
    private Context mContext;
    private Shader mTriangleShader;
    private int mScreenWidth;
    private int mScreenHeight;
    private int mVertexCount;

    public TriangleRender(Context context) {
        mContext = context;
        Size screenSize = Utils.getScreenResolution(context);
        mScreenWidth = screenSize.getWidth();
        mScreenHeight = screenSize.getHeight();
    }

    @Override
    public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {
        initVertex();
        mTriangleShader = new Shader(mContext, "triangle.vert", "triangle.frag");
    }

    @Override
    public void onSurfaceChanged(GL10 gl10, int i, int i1) {

    }

    @Override
    public void onDrawFrame(GL10 gl10) {
        GLES30.glViewport(0, 0, mScreenWidth, mScreenHeight);

        GLES30.glClearColor(1f, 1f, 1f, 0f);
        GLES30.glClear(GLES20.GL_COLOR_BUFFER_BIT);

        mTriangleShader.use();
        GLES30.glBindVertexArray(mVAO[0]);
        GLES30.glDrawArrays(GLES20.GL_TRIANGLES, 0, mVertexCount);
    }

    private void initVertex() {
        Log.d(TAG, "initVertex: ");

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
        GLES30.glGenVertexArrays(1, mVAO, 0);
        GLES30.glBindVertexArray(mVAO[0]);

        GLES30.glGenBuffers(1, vbo, 0);
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vbo[0]);
        int size = vertexBuffer.capacity() * FLOAT_SIZE;
        GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, size, vertexBuffer, GLES20.GL_STATIC_DRAW);

        GLES30.glEnableVertexAttribArray(0);
        GLES30.glVertexAttribPointer(0, 3, GLES30.GL_FLOAT, true, 6 * FLOAT_SIZE, 0);
        GLES30.glEnableVertexAttribArray(1);
        GLES30.glVertexAttribPointer(1, 3, GLES30.GL_FLOAT, true, 6 * FLOAT_SIZE, 3 * FLOAT_SIZE);
    }
}
