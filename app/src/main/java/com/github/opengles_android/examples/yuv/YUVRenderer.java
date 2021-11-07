package com.github.opengles_android.examples.yuv;

import android.app.Activity;
import android.content.res.AssetManager;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;

import androidx.annotation.NonNull;

public class YUVRenderer implements SurfaceHolder.Callback, Runnable {
    static {
        System.loadLibrary("yuv_renderer");
    }

    private final static String TAG = "YUVRenderer";

    private Thread mGLThread;
    private Surface mSurface;
    private Activity mActivity;
    private int mSurfaceWidth;
    private int mSurfaceHeight;
    private boolean mbStarted = false;

    public YUVRenderer(Activity activity) {
        mActivity = activity;
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {
        Log.e(TAG, "surfaceCreated");

        mbStarted = false;
        mSurface = holder.getSurface();
        mGLThread = new Thread(this);
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        Log.e(TAG, "surfaceChanged: width: " + width + ", height: " + height);

        mSurfaceWidth = width;
        mSurfaceHeight = height;

        if (mbStarted) {
            onSurfaceChanged(width, height);
        } else {
            mGLThread.start();
            mbStarted = true;
        }
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
        Log.e(TAG, "surfaceDestroyed");

        shutDown();

    }

    @Override
    public void run() {
        initRenderer(mSurface, mSurfaceWidth, mSurfaceHeight, mActivity.getAssets());
    }

    private native void initRenderer(Surface surface, int surfaceWidth, int surfaceHeight, AssetManager assetManager);
    private native void onSurfaceChanged(int width, int height);
    private native void shutDown();
}
