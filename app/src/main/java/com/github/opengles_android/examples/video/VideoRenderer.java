package com.github.opengles_android.examples.video;

import android.app.Activity;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;

import java.io.FileDescriptor;
import java.io.IOException;

import androidx.annotation.NonNull;

public class VideoRenderer implements SurfaceHolder.Callback, Runnable {
    static {
        System.loadLibrary("video_renderer");
    }

    private final static String TAG = "VideoRenderer";

    private Thread mGLThread;
    private Surface mSurface;
    private Activity mActivity;
    private int mSurfaceWidth;
    private int mSurfaceHeight;
    private boolean mbShutDown = false;
    private long mLastFrameTime = 0;

    public VideoRenderer(Activity activity) {
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
        initRenderer(mSurface, mSurfaceWidth, mSurfaceHeight, mActivity.getAssets());
        initVideoDecoder();
        mLastFrameTime = System.currentTimeMillis();

        while (!mbShutDown) {
            onDrawFrame();

            long curTime = System.currentTimeMillis();
            long frameTime = curTime - mLastFrameTime;
            mLastFrameTime = curTime;

            Log.d(TAG, "run, frameTime: " + frameTime);
        }

        Log.e(TAG, "run, exit.");

        release();
    }

    private native void initRenderer(Surface surface, int surfaceWidth, int surfaceHeight, AssetManager assetManager);
    private native void onDrawFrame();
    private native void initVideoDecoder();
    private native void release();
}
