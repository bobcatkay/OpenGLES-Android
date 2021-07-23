package com.github.opengles_android.examples.triangle;

import android.opengl.GLSurfaceView;
import android.os.Bundle;

import com.github.opengles_android.common.FullScreenActivity;
import com.github.opengles_android.databinding.ActivityTriangleBinding;

public class TriangleActivity extends FullScreenActivity {

    private TriangleRender mTriangleRender;
    private ActivityTriangleBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityTriangleBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        GLSurfaceView glSurfaceView = binding.glSurfaceView;
        glSurfaceView.setEGLContextClientVersion(3);

        mTriangleRender = new TriangleRender(getApplicationContext());
        glSurfaceView.setRenderer(mTriangleRender);
    }
}