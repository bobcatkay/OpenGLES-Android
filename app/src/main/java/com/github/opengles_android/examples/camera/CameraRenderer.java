package com.github.opengles_android.examples.camera;

import android.app.Activity;
import android.content.res.AssetManager;
import android.graphics.Rect;
import android.hardware.HardwareBuffer;
import android.media.Image;
import android.opengl.EGL14;
import android.os.Build;
import android.util.Log;
import android.util.Size;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

import com.github.opengles_android.R;
import com.github.opengles_android.common.Utils;

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
    private int mScreenWidth;
    private int mScreenHeight;
    private boolean mbConfigurationChanged = false;
    private SurfaceHolder mSurfaceHolder = null;
    private int mCount = 2;
    private final SurfaceView mSurfaceView;

    @RequiresApi(api = Build.VERSION_CODES.Q)
    public CameraRenderer(Activity activity) {
        mActivity = activity;
        mSurfaceView = mActivity.findViewById(R.id.camera_surface_view);
        Size screenResolution = Utils.getScreenResolution(activity);
        mScreenWidth = screenResolution.getWidth();
        mScreenHeight = screenResolution.getHeight();
        mSurfaceView.addOnLayoutChangeListener(new View.OnLayoutChangeListener() {
            @Override
            public void onLayoutChange(View v, int left, int top, int right, int bottom, int oldLeft, int oldTop, int oldRight, int oldBottom) {
                Log.e(TAG, "onLayoutChange: ");

//                synchronized (CameraRenderer.this) {
//                    mbConfigurationChanged = true;
//                }
            }
        });
    }

    @RequiresApi(api = Build.VERSION_CODES.R)
    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {
        Log.e(TAG, "surfaceCreated");

        mbShutDown = false;
        mbStarted = false;
        mSurface = holder.getSurface();
        mGLThread = new Thread(this);
        mRotation = mActivity.getDisplay().getRotation();
        mSurfaceHolder = holder;
    }

    private Rect mSurfaceFrame;
    private boolean mbStarted = false;
    private int mRotation = 0;
    private int mViewWidth = 0;
    @RequiresApi(api = Build.VERSION_CODES.R)
    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        Log.e(TAG, "surfaceChanged: width: " + width + ", height: " + height + ", rotation: " + mRotation + ", tid: " + Thread.currentThread().getId());

        int rotation = mActivity.getDisplay().getRotation();
        //onSurfaceChanged(width, height, rotation);
        mSurfaceHeight = height;
        mSurfaceWidth = width;

        if (!mbStarted) {
            mGLThread.start();
            mbStarted = true;
        }

        if (mCount > 2) {
            mCount = 0;
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
        mLastFrameTime = System.currentTimeMillis();

        while (!mbShutDown) {
            Buffer buffer = mBufferQueue.poll();

            if (null != buffer) {
                mCurrentBuffer = buffer;
            }

            int rotation = mActivity.getDisplay().getRotation();
            int viewWidth = mSurfaceView.getWidth();

            if (rotation != mRotation) {
                Log.d(TAG, "run, mRotation change, rotation: " + rotation + ", surfaceFrame: " + surfaceFrame.toShortString());
                mRotation = rotation;
                mViewWidth = viewWidth;
                //onSurfaceChanged(mScreenWidth, mScreenHeight, rotation);
            }


            synchronized (this) {
                if (mbConfigurationChanged) {
                    Log.d(TAG, "run, mbConfigurationChanged, rotation: " + rotation + ", surfaceFrame: " + surfaceFrame.toShortString());

                    onSurfaceChanged(mScreenWidth, mScreenHeight, rotation);
                    mbConfigurationChanged = false;
                }
            }

            if (null != mCurrentBuffer) {
                mCount ++;
                onDrawFrame(mCurrentBuffer.mHardwareBuffer, mCurrentBuffer.mWidth, mCurrentBuffer.mHeight);
            }

            long curTime = System.currentTimeMillis();
            long frameTime = curTime - mLastFrameTime;
            mLastFrameTime = curTime;

            //Log.d(TAG, "run, frameTime: " + frameTime + ", tid: " + Thread.currentThread().getId());
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

    public synchronized void onConfigurationChange() {
        mbConfigurationChanged = true;
    }

    private native void init(Surface surface, int surfaceWidth, int surfaceHeight, AssetManager assetManager);
    private native void onDrawFrame(HardwareBuffer buffer, int width, int height);
    private native void onSurfaceChanged(int width, int height, int rotation);
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
