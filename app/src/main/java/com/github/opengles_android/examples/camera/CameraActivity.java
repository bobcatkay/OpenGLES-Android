package com.github.opengles_android.examples.camera;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.SurfaceView;

import com.github.opengles_android.R;
import com.github.opengles_android.common.FullScreenActivity;

public class CameraActivity extends FullScreenActivity {

    private int CODE_PERMISSION_CAMERA;
    private CameraController mCameraController;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camera);

        SurfaceView cameraView = findViewById(R.id.camera_surface_view);
        CameraRenderer cameraRenderer = new CameraRenderer();
        cameraView.getHolder().addCallback(cameraRenderer);

        checkPermission();
    }

    private void initCamera() {
        mCameraController = new CameraController(this);
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