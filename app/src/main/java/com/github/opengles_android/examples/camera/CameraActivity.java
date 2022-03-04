package com.github.opengles_android.examples.camera;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceView;
import android.view.WindowManager;

import com.github.opengles_android.R;
import com.github.opengles_android.common.FullScreenActivity;

public class CameraActivity extends FullScreenActivity {

    private int CODE_PERMISSION_CAMERA;
    private CameraController mCameraController;
    private CameraRenderer mCameraRenderer;
    private GLSurfaceView mCameraView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camera);

        WindowManager.LayoutParams windowAttributes = getWindow().getAttributes();
        windowAttributes.rotationAnimation = WindowManager.LayoutParams.ROTATION_ANIMATION_SEAMLESS;
        getWindow().setAttributes(windowAttributes);

        mCameraView = findViewById(R.id.camera_surface_view);
        mCameraView.setEGLContextClientVersion(3);
        mCameraRenderer = new CameraRenderer(this);
        mCameraView.setRenderer(mCameraRenderer);
        mCameraView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        checkPermission();
    }

    public GLSurfaceView getSurfaceView() {
        return mCameraView;
    }

    @Override
    public void onConfigurationChanged(@NonNull Configuration newConfig) {
        mCameraRenderer.onConfigurationChanging();
        super.onConfigurationChanged(newConfig);


        Log.e("CameraActivity", "onConfigurationChanged: " );
    }

    @Override
    public boolean isChangingConfigurations() {
        Log.e("CameraActivity", "isChangingConfigurations: ");

        return super.isChangingConfigurations();
    }

    private void initCamera() {
        mCameraController = new CameraController(this);
        mCameraController.setImageReceivedListener(mCameraRenderer);
        mCameraController.init();
    }

    public void checkPermission() {
        int p2 = ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA);

        if (p2 != PackageManager.PERMISSION_GRANTED) {
            CODE_PERMISSION_CAMERA = 101;
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.CAMERA}, CODE_PERMISSION_CAMERA);
        } else {
            initCamera();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        if (CODE_PERMISSION_CAMERA == requestCode) {
            if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                initCamera();
            } else {
                onBackPressed();
            }
        }
    }

    @Override
    protected void onResume() {
        super.onResume();

        if (null != mCameraController) {
            mCameraController.onResume();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();

        if (null != mCameraController) {
            mCameraController.onPause();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if (null != mCameraController) {
            mCameraController.onDestroy();
        }
    }
}