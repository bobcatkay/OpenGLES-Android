package com.github.opengles_android.examples.video;

import android.content.Context;
import android.content.res.AssetManager;
import android.os.Environment;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import com.github.opengles_android.common.Utils;

import java.io.File;
import androidx.annotation.NonNull;

public class VideoView extends SurfaceView implements SurfaceHolder.Callback {
    static {
        System.loadLibrary("video_renderer");
    }

    private final static String TAG = "VideoRenderer";

    private Thread mVideoPlayThread;
    private Surface mSurface;
    private int mSurfaceWidth;
    private int mSurfaceHeight;

    public VideoView(Context context) {
        this(context, null);
    }

    public VideoView(Context context, AttributeSet attrs) {
        super(context, attrs);

        getHolder().addCallback(this);
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {
        Log.e(TAG, "surfaceCreated");

        mSurface = holder.getSurface();
        mVideoPlayThread = new VideoPlayThread();
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        Log.e(TAG, "surfaceChanged, width: " + width + ", height: " + height);

        mSurfaceWidth = width;
        mSurfaceHeight = height;
        mVideoPlayThread.start();
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
        Log.e(TAG, "surfaceDestroyed");

        shutdown();
    }

    class VideoPlayThread extends Thread{

        @Override
        public void run() {
            String videoPath = prepareFile();

            if (null == videoPath) {
                return;
            }

            initRenderer(mSurface, mSurfaceWidth, mSurfaceHeight, getContext().getAssets());
            initDecoder(videoPath);
        }
    }

    private String prepareFile() {
        File filesDir = getContext().getExternalFilesDir(Environment.DIRECTORY_MOVIES);
        String path = String.format("%s/%s", filesDir.getAbsolutePath(), "demo.mp4");

        if (new File(path).exists()) {
            return path;
        }

        if (!Utils.copyAsset(getContext().getAssets(), "demo.mp4", path)){
            Log.e(TAG, "run, copy file failed!");

            return null;
        }

        return path;
    }

    private native void initDecoder(String videoPath);
    private native void initRenderer(Surface surface, int surfaceWidth, int surfaceHeight, AssetManager assetManager);
    private native void shutdown();
}
