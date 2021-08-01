package com.github.opengles_android.examples.bitmap;

import android.content.Context;
import android.opengl.GLES20;
import android.opengl.GLES30;
import android.opengl.GLSurfaceView;
import android.renderscript.Matrix4f;
import android.util.Log;

import com.github.opengles_android.common.Shader;
import com.github.opengles_android.common.Texture;
import com.github.opengles_android.common.Utils;

import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class BitmapRender implements GLSurfaceView.Renderer {
    private final static String TAG = "BitmapRender";

    private final static int FLOAT_SIZE = Float.SIZE / Byte.SIZE;
    private int[] mVAO;
    private Context mContext;
    private Shader mShader;
    private int mSurfaceWidth;
    private int mSurfaceHeight;
    private int mVertexCount;
    private Matrix4f mTransform = new Matrix4f();
    private Matrix4f mProjectionMatrix = new Matrix4f();;
    private Texture mTexture;

    public BitmapRender(Context context) {
        mContext = context;
        mTransform.rotate(180.0f, 0, 0, 1.0f);
    }

    @Override
    public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {
        initVertex();
        mShader = new Shader(mContext, "common.vert", "texture2d.frag");
        mTexture = Texture.loadFromAssets(mContext, "test.jpeg", Texture.Type.RGBA);
    }

    @Override
    public void onSurfaceChanged(GL10 gl10, int width, int height) {
        mSurfaceWidth = width;
        mSurfaceHeight = height;

        float aspectRatio = width > height ?
                (float) width / (float) height :
                (float) height / (float) width;

        // 1. left：x的最小值
        // 2. right：x的最大值
        // 3. bottom：y的最小值
        // 4. top：y的最大值
        // 5. near：z的最小值
        // 6. far：z的最大值
        if (width > height) {
            // 横屏
            mProjectionMatrix.loadOrtho(-aspectRatio, aspectRatio, -1f, 1f, -1f, 1f);
            float scaleX =(float) mTexture.getWidth()/ mTexture.getHeight();
            mTransform.scale(scaleX, 1.0f, 1.0f);
        } else {
            // 竖屏or正方形
            mProjectionMatrix.loadOrtho(-1f, 1f, -aspectRatio, aspectRatio, -1f, 1f);

            float scaleY = (float) mTexture.getHeight() / mTexture.getWidth();
            mTransform.scale(1.0f, scaleY, 1.0f);
        }
    }

    @Override
    public void onDrawFrame(GL10 gl10) {
        GLES30.glViewport(0, 0, mSurfaceWidth, mSurfaceHeight);

        GLES30.glClearColor(1f, 1f, 1f, 1f);
        GLES30.glClear(GLES20.GL_COLOR_BUFFER_BIT);

        mShader.use();
        mShader.setMat4("uTransform", mTransform.getArray());
        mShader.setMat4("uProjection", mProjectionMatrix.getArray());
        mShader.setInt("uTexture", 0);
        Utils.checkGLError();
        mTexture.use(0);
        Utils.checkGLError();
        GLES30.glBindVertexArray(mVAO[0]);
        GLES30.glDrawArrays(GLES20.GL_TRIANGLES, 0, mVertexCount);
        Utils.checkGLError();
    }

    private void initVertex() {
        Log.d(TAG, "initVertex: ");

        float[] vertices = Utils.genQuadVertieces();
        FloatBuffer vertexBuffer = Utils.floatArrayToBuffer(vertices);
        mVertexCount = vertices.length / 5;

        mVAO = new int[1];
        int[] vbo = new int[1];
        GLES30.glGenVertexArrays(1, mVAO, 0);
        GLES30.glBindVertexArray(mVAO[0]);

        GLES30.glGenBuffers(1, vbo, 0);
        GLES30.glBindBuffer(GLES30.GL_ARRAY_BUFFER, vbo[0]);
        int size = vertexBuffer.capacity() * FLOAT_SIZE;
        GLES30.glBufferData(GLES30.GL_ARRAY_BUFFER, size, vertexBuffer, GLES20.GL_STATIC_DRAW);

        GLES30.glEnableVertexAttribArray(0);
        GLES30.glVertexAttribPointer(0, 3, GLES30.GL_FLOAT, false, 5 * FLOAT_SIZE, 0);
        GLES30.glEnableVertexAttribArray(1);
        GLES30.glVertexAttribPointer(1, 2, GLES30.GL_FLOAT, false, 5 * FLOAT_SIZE, 3 * FLOAT_SIZE);
    }
}
