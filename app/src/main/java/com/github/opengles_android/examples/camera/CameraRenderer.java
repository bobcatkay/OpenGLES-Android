package com.github.opengles_android.examples.camera;

import android.app.Activity;
import android.content.res.AssetManager;
import android.hardware.HardwareBuffer;
import android.media.Image;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;

import java.util.concurrent.ConcurrentLinkedQueue;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;

public class CameraRenderer implements SurfaceHolder.Callback, Runnable, OnImageReceivedListener {
    static {
        System.loadLibrary("camera_renderer");
    }

    private final static String TAG = "CameraRenderer";

    private Thread mGLThread;
    private Surface mSurface;
    private Activity mActivity;
    private int mSurfaceWidth;
    private int mSurfaceHeight;
    private boolean mbShutDown = false;
    private ConcurrentLinkedQueue<Buffer> mBufferQueue = new ConcurrentLinkedQueue<>();
    private Buffer mCurrentBuffer;
    private long mLastFrameTime = 0;

    public CameraRenderer(Activity activity) {
        mActivity = activity;
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {
        Log.e(TAG, "surfaceCreated");

        mbShutDown = false;
        mSurface = holder.getSurface();
        mGLThread = new Thread(this);
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        Log.e(TAG, "surfaceChanged: width: " + width + ", height: " + height);

        mSurfaceWidth = width;
        mSurfaceHeight = height;
        mGLThread.start();
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
        Log.e(TAG, "surfaceDestroyed");

        mbShutDown = true;
    }

    @Override
    public void run() {
        init(mSurface, mSurfaceWidth, mSurfaceHeight, mActivity.getAssets());
        mLastFrameTime = System.currentTimeMillis();

        while (!mbShutDown) {
            Buffer buffer = mBufferQueue.poll();

            if (null != buffer) {
                mCurrentBuffer = buffer;
            }

            if (null != mCurrentBuffer) {
                onDrawFrame(mCurrentBuffer.mHardwareBuffer, mCurrentBuffer.mWidth, mCurrentBuffer.mHeight);
            }

            long curTime = System.currentTimeMillis();
            long frameTime = curTime - mLastFrameTime;
            mLastFrameTime = curTime;

            Log.d(TAG, "run, frameTime: " + frameTime);
        }

        Log.e(TAG, "run, exit.");

        release();
    }

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

        Log.d(TAG, "onImageReceived, image size: " + width + "x" + height + ", format: " + image.getFormat());
    }

    private native void init(Surface surface, int surfaceWidth, int surfaceHeight, AssetManager assetManager);
    private native void onDrawFrame(HardwareBuffer buffer, int width, int height);
    private native void release();

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
