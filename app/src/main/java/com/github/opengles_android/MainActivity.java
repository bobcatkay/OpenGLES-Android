package com.github.opengles_android;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;

import com.github.opengles_android.databinding.ActivityMainBinding;
import com.github.opengles_android.examples.bitmap.BitmapActivity;
import com.github.opengles_android.examples.camera.CameraActivity;
import com.github.opengles_android.examples.native_render.NativeRenderActivity;
import com.github.opengles_android.examples.triangle.TriangleActivity;
import com.github.opengles_android.examples.yuv.YUVActivity;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity implements View.OnClickListener{
    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        binding.buttonTriangle.setOnClickListener(this);
        binding.buttonBitmap.setOnClickListener(this);
        binding.buttonNativeRender.setOnClickListener(this);
        binding.buttonCamera.setOnClickListener(this);
        binding.buttonVideo.setOnClickListener(this);

    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.button_triangle:
                goToActivity(TriangleActivity.class);
                break;

            case R.id.button_bitmap:
                goToActivity(BitmapActivity.class);
                break;

            case R.id.button_native_render:
                goToActivity(NativeRenderActivity.class);
                break;

            case R.id.button_camera:
                goToActivity(CameraActivity.class);
                break;

            case R.id.button_video:
                goToActivity(YUVActivity.class);
                break;
        }
    }

    private void goToActivity(Class<?> cls) {
        Intent intent = new Intent(MainActivity.this, cls);
        startActivity(intent);
    }
}