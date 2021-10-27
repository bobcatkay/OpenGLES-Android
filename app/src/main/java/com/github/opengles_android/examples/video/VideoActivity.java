package com.github.opengles_android.examples.video;


import android.os.Bundle;
import android.view.SurfaceView;

import com.github.opengles_android.R;
import com.github.opengles_android.common.FullScreenActivity;

public class VideoActivity extends FullScreenActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video);

        SurfaceView cameraView = findViewById(R.id.video_surface_view);
        VideoRenderer videoRenderer = new VideoRenderer(this);
        cameraView.getHolder().addCallback(videoRenderer);
    }
}