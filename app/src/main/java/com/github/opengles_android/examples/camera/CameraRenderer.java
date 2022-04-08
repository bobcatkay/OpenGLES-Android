package com.github.opengles_android.examples.camera;

import android.app.Activity;
import android.content.res.AssetManager;
import android.hardware.HardwareBuffer;
import android.media.Image;
import android.os.Build;
import android.os.ConditionVariable;
import android.os.Environment;
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
    private ConditionVariable mLock = new ConditionVariable(false);
    private boolean mbStarted = false;
    private int mRotation;
    private boolean mbSurfaceChanged = false;
    private boolean mbConfigurationChanged = false;

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

    public void onConfigurationChanged() {
        synchronized (this) {
            mbConfigurationChanged = true;
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.R)
    private int getDisplayRotation() {
        return mActivity.getDisplay().getRotation();
    }

    @RequiresApi(api = Build.VERSION_CODES.R)
    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        Log.e(TAG, "surfaceChanged: width: " + width + ", height: " + height);

        mSurfaceWidth = width;
        mSurfaceHeight = height;
        int rotation = getDisplayRotation();
        //onSurfaceChanged(mSurfaceWidth, mSurfaceHeight, rotation);


        synchronized (this) {
            mbSurfaceChanged = true;
        }

        if (!mbStarted) {
            mGLThread.start();
            mbStarted = true;
        }
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
        Log.e(TAG, "surfaceDestroyed");

        mbShutDown = true;
    }

    @RequiresApi(api = Build.VERSION_CODES.R)
    @Override
    public void run() {
        init(mSurface, mSurfaceWidth, mSurfaceHeight, mActivity.getAssets());
        setTextureStoreDir(mActivity.getExternalFilesDir(Environment.DIRECTORY_PICTURES).getAbsolutePath());
        mLastFrameTime = System.currentTimeMillis();

        while (!mbShutDown) {
            mLock.block();
            mCurrentBuffer = mBufferQueue.poll();

            int rotation = getDisplayRotation();

            if (mRotation != rotation) {
                beginDump();
                mRotation = rotation;
            }

            synchronized (this) {
                if (mbSurfaceChanged) {
                    onSurfaceChanged(mSurfaceWidth, mSurfaceHeight, rotation);
                    mbSurfaceChanged = false;
                }
            }

            onDrawFrame(mCurrentBuffer.mHardwareBuffer, mCurrentBuffer.mWidth, mCurrentBuffer.mHeight);

            long curTime = System.currentTimeMillis();
            long frameTime = curTime - mLastFrameTime;
            mLastFrameTime = curTime;

            Log.d(TAG, "run, frameTime: " + frameTime);

            mLock.close();
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
        mLock.open();

        Log.d(TAG, "onImageReceived, image size: " + width + "x" + height + ", format: " + image.getFormat());
    }

    private native void init(Surface surface, int surfaceWidth, int surfaceHeight, AssetManager assetManager);
    private native void onDrawFrame(HardwareBuffer buffer, int width, int height);
    private native void onSurfaceChanged(int width, int height, int rotation);
    private native void release();
    private native void setTextureStoreDir(String dir);
    private native void beginDump();

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
