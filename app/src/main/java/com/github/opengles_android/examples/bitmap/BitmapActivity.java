package com.github.opengles_android.examples.bitmap;

import android.opengl.GLSurfaceView;
import android.os.Bundle;

import com.github.opengles_android.R;
import com.github.opengles_android.common.FullScreenActivity;

public class BitmapActivity extends FullScreenActivity {

    private BitmapRender mBitmapRender;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_bitmap);

        GLSurfaceView glSurfaceView = findViewById(R.id.gl_surface_view);
        glSurfaceView.setEGLContextClientVersion(3);

        mBitmapRender = new BitmapRender(getApplicationContext());
        glSurfaceView.setRenderer(mBitmapRender);

    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        mBitmapRender.destroy();
    }
}