package com.github.opengles_android.examples.video;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.os.ParcelFileDescriptor;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;

import java.io.File;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

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
    private ParcelFileDescriptor mVideoFileFD = null;

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

        if (null != mVideoFileFD) {
            mGLThread.start();
        }
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
        Log.e(TAG, "surfaceDestroyed");

        mbShutDown = true;
    }

    public void setVideoFD(ParcelFileDescriptor fd) {
        mVideoFileFD = fd;
    }

    @Override
    public void run() {
        initRenderer(mSurface, mSurfaceWidth, mSurfaceHeight, mActivity.getAssets(), mVideoFileFD.getFd());

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

        try {
            mVideoFileFD.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private native void initRenderer(Surface surface, int surfaceWidth, int surfaceHeight, AssetManager assetManager, int videoFileFD);
    private native void onDrawFrame();
    private native void release();
}
