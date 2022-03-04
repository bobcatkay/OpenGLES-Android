package com.github.opengles_android.examples.camera;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetManager;
import android.hardware.HardwareBuffer;
import android.media.Image;
import android.opengl.GLES30.*;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.renderscript.Matrix4f;
import android.util.Log;
import android.util.Size;
import android.view.Surface;
import android.view.SurfaceHolder;

import com.github.opengles_android.common.Shader;
import com.github.opengles_android.common.Texture;
import com.github.opengles_android.common.Utils;

import java.nio.FloatBuffer;
import java.util.concurrent.ConcurrentLinkedQueue;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;

import static android.opengl.GLES11Ext.GL_TEXTURE_EXTERNAL_OES;
import static android.opengl.GLES20.*;
import static android.opengl.GLES30.*;

public class CameraRenderer implements GLSurfaceView.Renderer, OnImageReceivedListener {
    static {
        System.loadLibrary("camera_renderer");
    }

    private final static String TAG = "CameraRenderer";

    private final static int FLOAT_SIZE = Float.SIZE / Byte.SIZE;
    private Surface mSurface;
    private CameraActivity mActivity;
    private int mSurfaceWidth;
    private int mSurfaceHeight;
    private boolean mbShutDown = false;
    private ConcurrentLinkedQueue<Buffer> mBufferQueue = new ConcurrentLinkedQueue<>();
    private Buffer mCurrentBuffer;
    private long mLastFrameTime = 0;
    private int[] mVAO;
    private Shader mShader;
    private int mTextureId;
    private int mVertexCount;
    private Matrix4f mTransform = new Matrix4f();
    private Matrix4f mProjectionMatrix = new Matrix4f();
    private boolean mbIsConfigurationChanging = false;
    private int mRotation;

    public CameraRenderer(CameraActivity activity) {
        mActivity = activity;
    }

//    @Override
//    public void run() {
//        init(mSurface, mSurfaceWidth, mSurfaceHeight, mActivity.getAssets());
//        mLastFrameTime = System.currentTimeMillis();
//
//        while (!mbShutDown) {
//            Buffer buffer = mBufferQueue.poll();
//
//            if (null != buffer) {
//                mCurrentBuffer = buffer;
//            }
//
//            if (null != mCurrentBuffer) {
//                onDrawFrame(mCurrentBuffer.mHardwareBuffer, mCurrentBuffer.mWidth, mCurrentBuffer.mHeight);
//            }
//
//            long curTime = System.currentTimeMillis();
//            long frameTime = curTime - mLastFrameTime;
//            mLastFrameTime = curTime;
//
//            Log.d(TAG, "run, frameTime: " + frameTime);
//        }
//
//        Log.e(TAG, "run, exit.");
//
//        release();
//    }

    @RequiresApi(api = Build.VERSION_CODES.P)
    @Override
    public void onImageReceived(Image image) {
        HardwareBuffer hardwareBuffer = image.getHardwareBuffer();
        int width = image.getWidth();
        int height = image.getHeight();
        Buffer buffer = new Buffer(hardwareBuffer, height, width);

        // Drop frame if GPU performance is very low.
        if (mBufferQueue.size() > 3) {
            mBufferQueue.poll();
        }

        mBufferQueue.add(buffer);

        synchronized (this) {
            if (!mbIsConfigurationChanging) {
                mActivity.getSurfaceView().requestRender();
            }
        }

        //Log.d(TAG, "onImageReceived, image size: " + width + "x" + height + ", format: " + image.getFormat());
    }

    private native void init(Surface surface, int surfaceWidth, int surfaceHeight, AssetManager assetManager);
    private native void onDrawFrame(HardwareBuffer buffer, int width, int height);
    private native void release();
    private native void bindHardwareBuffer(int texId, HardwareBuffer buffer, int width, int height);

    public synchronized void onConfigurationChanging() {
        mbIsConfigurationChanging = true;
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        initVertex();
        mShader = new Shader(mActivity, "common.vert", "texture2d_oes.frag");

        int[] texId = new int[1];
        glGenTextures(1, texId, 0);
        mTextureId = texId[0];
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, mTextureId);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        mSurfaceWidth = width;
        mSurfaceHeight = height;

        Size screenResolution = Utils.getScreenResolution(mActivity);
        mRotation = mActivity.getWindowManager().getDefaultDisplay().getRotation();
        Log.e(TAG, "onSurfaceChanged: width: " + width + ", height: " + height + ", mRotation: " + mRotation + ", screenResolution: " + screenResolution);

        mProjectionMatrix.loadIdentity();
        float aspectRatio = width > height ?
                (float) width / (float) height :
                (float) height / (float) width;

        if (width > height) {
            mProjectionMatrix.loadOrtho(-aspectRatio, aspectRatio, -1f, 1f, -1f, 1f);
        } else {
            mProjectionMatrix.loadOrtho(-1f, 1f, -aspectRatio, aspectRatio, -1f, 1f);
        }

        //onDrawFrame(null);
        mbIsConfigurationChanging = false;
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        Buffer buffer = mBufferQueue.poll();

        if (null != buffer) {
            mCurrentBuffer = buffer;
        }

        updateMatrix();

        glViewport(0, 0, mSurfaceWidth, mSurfaceHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f,0.0f,0.0f,1.0f);

        mShader.use();
        mShader.setInt("uTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, mTextureId);

        glBindVertexArray(mVAO[0]);

        glDrawArrays(GL_TRIANGLES, 0, mVertexCount);
    }

    private synchronized void updateMatrix() {
        if (null != mCurrentBuffer) {
            int texWidth = mCurrentBuffer.mWidth;
            int texHeight = mCurrentBuffer.mHeight;
            bindHardwareBuffer(mTextureId, mCurrentBuffer.mHardwareBuffer, texWidth, texHeight);

            mTransform.loadIdentity();
            float scaleY = 1.0f;
            float scaleX = 1.0f;

            if (0 == mRotation) {
                if (mSurfaceHeight > mSurfaceWidth) {
                    scaleY = (float) texHeight / texWidth;
                } else {
                    scaleX = (float) texWidth / texHeight;
                }
            } else {
                if (mSurfaceHeight > mSurfaceWidth) {
                    scaleY = (float) texWidth / texHeight;
                } else {
                    scaleX = (float) texHeight / texWidth;
                }
            }

            mTransform.scale(scaleX, scaleY, 1.0f);

            if (0 == mRotation) {
                mTransform.rotate(270.0f, 0, 0, 1.0f);
                mTransform.scale(1.0f, -1.0f, 1.0f);
            } else if (1 == mRotation) {
                mTransform.rotate(180.0f, 0, 0, 1.0f);
                mTransform.scale(-1.0f, 1.0f, 1.0f);
            } else {
                mTransform.scale(-1.0f, 1.0f, 1.0f);
            }

        }

        mShader.use();
        mShader.setMat4("uProjection", mProjectionMatrix.getArray());
        mShader.setMat4("uTransform", mTransform.getArray());
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

    static class Buffer {
        public HardwareBuffer mHardwareBuffer;
        public int mWidth;
        public int mHeight;

        public Buffer(HardwareBuffer hardwareBuffer, int width, int height) {
            mHardwareBuffer = hardwareBuffer;
            mWidth = width;
            mHeight = height;
        }
    }
}
