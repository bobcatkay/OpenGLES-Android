package com.github.opengles_android.examples.bitmap;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.renderscript.Matrix4f;
import android.util.Log;

import com.github.opengles_android.common.Shader;
import com.github.opengles_android.common.Texture;
import com.github.opengles_android.common.Utils;

import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import static android.opengl.GLES30.*;

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
    private Matrix4f mProjectionMatrix = new Matrix4f();
    private Texture mTexture;
    private float mInitialScaleX;
    private float mInitialScaleY;
    private float mTextureScale;

    public BitmapRender(Context context) {
        mContext = context;
    }

    @Override
    public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {
        initVertex();
        mShader = new Shader(mContext, "bitmap.vert", "texture2d.frag");
        mTexture = Texture.loadFromAssets(mContext, "test.jpg");
    }

    @Override
    public void onSurfaceChanged(GL10 gl10, int width, int height) {
        mSurfaceWidth = width;
        mSurfaceHeight = height;

        mProjectionMatrix.loadIdentity();
        float aspectRatio = width > height ?
                (float) width / (float) height :
                (float) height / (float) width;

        if (width > height) {
            mProjectionMatrix.loadOrtho(-aspectRatio, aspectRatio, -1f, 1f, -1f, 1f);
        } else {
            mProjectionMatrix.loadOrtho(-1f, 1f, -aspectRatio, aspectRatio, -1f, 1f);
        }

        int texWidth = mTexture.getWidth();
        int texHeight = mTexture.getHeight();
        float texRatio = (float) texHeight / texWidth;
        float targetHeight = width * texRatio;

        if (targetHeight <= height) {
            mInitialScaleX = (width <= height) ? 1.0f : ((float) width / height);
            mInitialScaleY = (width <= height) ? (targetHeight / width) : (targetHeight / height);
        } else {
            float targetWidth = (float) height / texRatio;
            mInitialScaleX = (width <= height) ? (targetWidth / width) : (targetWidth / height);
            mInitialScaleY = (width <= height) ? ((float) height / width) : 1.0f;
        }

        updateTransform(new TransformData());
    }

    @Override
    public void onDrawFrame(GL10 gl10) {
        glViewport(0, 0, mSurfaceWidth, mSurfaceHeight);

        glClearColor(1f, 1f, 1f, 1f);
        glClear(GL_COLOR_BUFFER_BIT);

        mShader.use();

        synchronized (this) {
            mShader.setMat4("uTransform", mTransform.getArray());
            mShader.setFloat("uTextureScale", mTextureScale);
        }

        mShader.setMat4("uProjection", mProjectionMatrix.getArray());
        mShader.setInt("uTexture", 0);
        Utils.checkGLError();
        mTexture.bind(0);
        Utils.checkGLError();
        glBindVertexArray(mVAO[0]);
        glDrawArrays(GL_TRIANGLES, 0, mVertexCount);
        Utils.checkGLError();
    }

    private void initVertex() {
        Log.d(TAG, "InitVertex: ");

        float[] vertices = Utils.genQuadVertieces();
        FloatBuffer vertexBuffer = Utils.floatArrayToBuffer(vertices);
        mVertexCount = vertices.length / 5;

        mVAO = new int[1];
        int[] vbo = new int[1];
        glGenVertexArrays(1, mVAO, 0);
        glBindVertexArray(mVAO[0]);

        glGenBuffers(1, vbo, 0);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        int size = vertexBuffer.capacity() * FLOAT_SIZE;
        glBufferData(GL_ARRAY_BUFFER, size, vertexBuffer, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, false, 5 * FLOAT_SIZE, 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, false, 5 * FLOAT_SIZE, 3 * FLOAT_SIZE);
    }

    public synchronized void updateTransform(TransformData data) {
        mTransform.loadIdentity();

        mTransform.translate(data.mTranslateX, data.mTranslateY, 0);
        mTransform.rotate(data.mRotateX, 1.0f, 0, 0);
        mTransform.rotate(data.mRotateY, 0, 1.0f, 0);
        mTransform.rotate(data.mRotateZ, 0, 0, 1.0f);
        mTransform.scale(data.mScaleX, data.mScaleY, 1.0f);

        mTransform.scale(mInitialScaleX, mInitialScaleY, 1.0f);
        mTextureScale = data.mTextureScale;
    }

    public void destroy() {
        glDeleteBuffers(1, mVAO, 0);
        mShader.release();
        mTexture.release();
    }
}
