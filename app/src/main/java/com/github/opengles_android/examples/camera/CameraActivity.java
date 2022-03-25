package com.github.opengles_android.examples.camera;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.graphics.RenderNode;
import android.os.Bundle;
import android.util.Log;
import android.util.Size;
import android.view.SurfaceView;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.RelativeLayout;

import com.github.opengles_android.R;
import com.github.opengles_android.common.FullScreenActivity;
import com.github.opengles_android.common.Utils;

public class CameraActivity extends FullScreenActivity {

    private int CODE_PERMISSION_CAMERA;
    private CameraController mCameraController;
    private CameraRenderer mCameraRenderer;
    private SurfaceView mCameraView;
    private Size mScreenResolution;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camera);

        mCameraView = findViewById(R.id.camera_surface_view);
        mCameraRenderer = new CameraRenderer(this);
        mCameraView.getHolder().addCallback(mCameraRenderer);
        mScreenResolution = Utils.getScreenResolution(this);
        mCameraView.getHolder().setFixedSize(mScreenResolution.getWidth(), mScreenResolution.getHeight());

        WindowManager.LayoutParams windowAttributes = getWindow().getAttributes();
        windowAttributes.rotationAnimation = WindowManager.LayoutParams.ROTATION_ANIMATION_SEAMLESS;
        getWindow().setAttributes(windowAttributes);

        checkPermission();
    }

    @Override
    public void onConfigurationChanged(@NonNull Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        mCameraRenderer.onConfigurationChange();

        Log.e("CameraActivity", "onConfigurationChanged, newConfig: " + newConfig.orientation);
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