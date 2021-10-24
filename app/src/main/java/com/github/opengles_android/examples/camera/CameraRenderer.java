package com.github.opengles_android.examples.camera;

import android.app.Activity;
import android.content.res.AssetManager;
import android.hardware.HardwareBuffer;
import android.media.Image;
import android.opengl.GLES11Ext;
import android.os.Build;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;

public class CameraRenderer implements SurfaceHolder.Callback, Runnable, OnImageReceivedListener {
    static {
        System.loadLibrary("renderer");
    }

    private final static String TAG = "CameraRenderer";

    private Thread mGLThread;
    private Surface mSurface;
    private Activity mActivity;
    private int mSurfaceWidth;
    private int mSurfaceHeight;

    public CameraRenderer(Activity activity) {
        mActivity = activity;
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {
        Log.e(TAG, "surfaceCreated");

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

        shutDown();
    }

    @Override
    public void run() {
        init(mSurface, mSurfaceWidth, mSurfaceHeight, mActivity.getAssets());
    }

    @RequiresApi(api = Build.VERSION_CODES.P)
    @Override
    public void onImageReceived(Image image) {
        HardwareBuffer hardwareBuffer = image.getHardwareBuffer();
        int width = image.getWidth();
        int height = image.getHeight();
        addBuffer(hardwareBuffer, width, height);
        hardwareBuffer.close();

        Log.d(TAG, "onImageReceived, image size: " + width + "x" + height + ", format: " + image.getFormat()
                + ", time: " + image.getTimestamp());
    }

    private native void init(Surface surface, int surfaceWidth, int surfaceHeight, AssetManager assetManager);
    private native void addBuffer(HardwareBuffer buffer, int width, int height);
    private native void shutDown();

}
