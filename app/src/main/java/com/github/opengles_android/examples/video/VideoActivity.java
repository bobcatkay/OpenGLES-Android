package com.github.opengles_android.examples.video;


import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.ParcelFileDescriptor;
import android.provider.MediaStore;
import android.util.Log;
import android.view.SurfaceView;

import com.github.opengles_android.R;
import com.github.opengles_android.common.FullScreenActivity;

import java.io.FileNotFoundException;
import java.io.IOException;

import androidx.annotation.Nullable;

public class VideoActivity extends FullScreenActivity {
    private final static String TAG = "VideoActivity";

    private int PICK_VIDEO_REQUEST = 0x200;
    private VideoRenderer mVideoRenderer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video);

        SurfaceView cameraView = findViewById(R.id.video_surface_view);
        mVideoRenderer = new VideoRenderer(this);
        cameraView.getHolder().addCallback(mVideoRenderer);

        //Intent i = new Intent(Intent.ACTION_PICK, MediaStore.Video.Media.EXTERNAL_CONTENT_URI);
        //startActivityForResult(i, PICK_VIDEO_REQUEST);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(resultCode, resultCode, data);

        if (requestCode == PICK_VIDEO_REQUEST && resultCode == RESULT_OK && null != data) {
            Uri selectedVideo = data.getData();
            try {
                ParcelFileDescriptor fileDescriptor = getContentResolver().openFileDescriptor(selectedVideo, "r");
                //int fd = fileDescriptor.getFd();
                mVideoRenderer.setVideoFD(fileDescriptor);
                //fileDescriptor.close();

                Log.d(TAG, "onActivityResult, fd: " + fileDescriptor.getFd());
            } catch (IOException e) {
                e.printStackTrace();
            }
        } else {
            finish();
        }
    }
}