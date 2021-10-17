package com.github.opengles_android.examples.native_render;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;

import com.github.opengles_android.R;
import com.github.opengles_android.common.FullScreenActivity;

public class NativeRenderActivity extends FullScreenActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_native_render);
    }
}