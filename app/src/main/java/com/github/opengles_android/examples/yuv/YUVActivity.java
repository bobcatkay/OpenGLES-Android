package com.github.opengles_android.examples.yuv;


import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.ParcelFileDescriptor;
import android.util.Log;
import android.view.SurfaceView;

import com.github.opengles_android.R;
import com.github.opengles_android.common.FullScreenActivity;

import java.io.IOException;

import androidx.annotation.Nullable;

public class YUVActivity extends FullScreenActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_yuv_renderer);

        SurfaceView cameraView = findViewById(R.id.video_surface_view);
        YUVRenderer yuvRenderer = new YUVRenderer(this);
        cameraView.getHolder().addCallback(yuvRenderer);
    }
}