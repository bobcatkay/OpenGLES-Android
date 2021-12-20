package com.github.opengles_android.examples.native_render;

import android.content.Context;
import android.content.res.AssetManager;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import com.github.opengles_android.common.Utils;

import androidx.annotation.NonNull;

public class NativeRenderView extends SurfaceView implements SurfaceHolder.Callback, Runnable {
    static {
        System.loadLibrary("native-lib");
    }

    private final static String TAG = "NativeRenderView";

    private Thread mGLThread;
    private Surface mSurface;
    private boolean mbStarted = false;

    public NativeRenderView(Context context) {
        this(context, null);
    }

    public NativeRenderView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);

    }

    public NativeRenderView(Context context, AttributeSet attrs, int defStyleAttr) {
        this(context, attrs, defStyleAttr, 0);
    }

    public NativeRenderView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);

        getHolder().addCallback(this);
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

        setSurfaceSize(width, height);

        if (!mbStarted) {
            mGLThread.start();
            mbStarted = true;
        }
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
        Log.e(TAG, "surfaceDestroyed");

        stop();
        mbStarted = false;
    }

    @Override
    public void run() {
        String vertexCode = Utils.getStringFromAssets(getContext(), "triangle.vert");
        String fragCode = Utils.getStringFromAssets(getContext(), "triangle.frag");
        init(mSurface, vertexCode, fragCode, getContext().getAssets());
    }

    private native void init(Surface surface, String vertexCode, String fragCode, AssetManager assetManager);

    private native void setSurfaceSize(int width, int height);

    private native void stop();
}
