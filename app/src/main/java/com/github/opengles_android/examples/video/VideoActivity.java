package com.github.opengles_android.examples.video;


import android.os.Bundle;

import com.github.opengles_android.R;
import com.github.opengles_android.common.FullScreenActivity;

public class VideoActivity extends FullScreenActivity {
    private final static String TAG = "VideoActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video);
    }
}