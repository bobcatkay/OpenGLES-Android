package com.github.opengles_android.examples.camera;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Service;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureRequest;
import android.media.Image;
import android.media.ImageReader;
import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.util.Log;
import android.util.Size;
import android.view.Surface;

import com.github.opengles_android.common.Utils;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;

public class CameraController {
    private static final String TAG = "CameraController";
    private static final int MSG_ON_CONFIGURED = 0x100;
    private static final int MSG_DO_PREV_CAPTURE = 0x101;
    private static final float RATIO = 4.0f / 3;
    private static final int MAX_SIZE = 1920;

    private Activity mActivity;
    private CameraManager mCameraManager;
    private HandlerThread mCameraThread;
    private HandlerThread mImageThread;
    private Handler mCameraHandler;
    private Handler mImageHandler;
    private CameraDevice mCameraDevice;
    private CameraCharacteristics mCameraCharacteristics;
    private String mCameraId;
    private Size mPreviewSize;
    private ImageReader mImageReader;
    private CaptureRequest mPrevCaptureRequest;
    private CameraCaptureSession mCaptureSession;
    private OnImageReceivedListener mImageReceivedListener;

    private CameraDevice.StateCallback mStateCallback = new CameraDevice.StateCallback() {
        @Override
        public void onOpened(@NonNull CameraDevice camera) {
            Log.d(TAG, "onOpened: ");
            mCameraDevice = camera;
            createPrevCaptureSession();
        }

        @Override
        public void onDisconnected(@NonNull CameraDevice camera) {
            Log.w(TAG, "onDisconnected: ");
        }

        @Override
        public void onError(@NonNull CameraDevice camera, int error) {
            Log.e(TAG, "onError: ");
        }
    };

    private CameraCaptureSession.StateCallback mSessionStateCallback = new CameraCaptureSession.StateCallback() {
        @Override
        public void onConfigured(@NonNull CameraCaptureSession session) {
            Log.d(TAG, "onConfigured: ");
            createCaptureRequest();
            mCaptureSession = session;
            mCameraHandler.sendEmptyMessage(MSG_ON_CONFIGURED);
        }

        @Override
        public void onConfigureFailed(@NonNull CameraCaptureSession session) {
            Log.e(TAG, "onConfigureFailed: ");
        }
    };

    private ImageReader.OnImageAvailableListener mOnImageAvailableListener = new ImageReader.OnImageAvailableListener() {
        @RequiresApi(api = Build.VERSION_CODES.P)
        @Override
        public void onImageAvailable(ImageReader reader) {
            Image image = reader.acquireNextImage();

            if (null != mImageReceivedListener) {
                mImageReceivedListener.onImageReceived(image);
            }

            image.close();
        }
    };

    public CameraController(Activity activity) {
        mActivity = activity;
    }

    public void init() {
        Log.d(TAG, "init");

        mCameraManager = (CameraManager) mActivity.getSystemService(Service.CAMERA_SERVICE);

        try {
            String[] idList = mCameraManager.getCameraIdList();

            for (String id : idList) {
                mCameraCharacteristics = mCameraManager.getCameraCharacteristics(id);
                if (mCameraCharacteristics.get(CameraCharacteristics.LENS_FACING) == CameraMetadata.LENS_FACING_BACK) {
                    mCameraId = id;

                    Log.d(TAG, "init, mBackCameraId: " + mCameraId);
                    break;
                }
            }

            mCameraId = "0";
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }

        initHandler();
        openCamera();
    }

    private void initHandler() {
        mCameraThread = new HandlerThread("CameraThread");
        mImageThread = new HandlerThread("mImageThread");
        mCameraThread.start();
        mCameraHandler = new Handler(mCameraThread.getLooper()) {
            @Override
            public void handleMessage(@NonNull Message msg) {
                switch (msg.what) {
                    case MSG_ON_CONFIGURED:
                        try {
                            CaptureRequest request[] = new CaptureRequest[] {mPrevCaptureRequest};
                            mCaptureSession.setRepeatingBurst(Arrays.asList(request.clone()), null, mImageHandler);
                        } catch (CameraAccessException e) {
                            e.printStackTrace();
                        }
                        break;

                    default:
                        break;

                }
            }
        };

        mImageThread.start();
        mImageHandler = new Handler(mImageThread.getLooper());
    }

    @SuppressLint("MissingPermission")
    private void openCamera() {
        Log.d(TAG, "openCamera");

        try {
            mCameraManager.openCamera(mCameraId, mStateCallback, mCameraHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private void createPrevCaptureSession() {
        Log.d(TAG, "createPrevCaptureSession");

        mPreviewSize = Utils.getOptimalCameraSize(mCameraCharacteristics, SurfaceTexture.class, MAX_SIZE, RATIO);

        if (null == mPreviewSize) {
            Log.e(TAG, "open, Failed to get preview size.");

            return;
        }

        Log.d(TAG, "createPrevCaptureSession, mPreviewSize: " + mPreviewSize.getWidth() + "x" + mPreviewSize.getHeight());

        mImageReader = ImageReader.newInstance(mPreviewSize.getWidth(), mPreviewSize.getHeight(), ImageFormat.YUV_420_888, 3);
        mImageReader.setOnImageAvailableListener(mOnImageAvailableListener, mCameraHandler);

        List<Surface> outputs = new ArrayList<>();
        outputs.add(mImageReader.getSurface());

        try {
            mCameraDevice.createCaptureSession(outputs, mSessionStateCallback, mCameraHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private void createCaptureRequest() {
        Log.d(TAG, "createCaptureRequest");

        try {
            CaptureRequest.Builder builder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_RECORD);
            builder.addTarget(mImageReader.getSurface());
            mPrevCaptureRequest = builder.build();
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    public void setImageReceivedListener(OnImageReceivedListener imageReceivedListener) {
        mImageReceivedListener = imageReceivedListener;
    }

    public void onResume() {
        if (null == mCameraManager) {
            init();
        }
    }

    public void onPause() {
        if (null != mCaptureSession) {
            try {
                mCaptureSession.stopRepeating();
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }
        }

        if (null != mCaptureSession) {
            mCaptureSession.close();
            mCaptureSession = null;
        }

        if (null != mCameraDevice) {
            mCameraDevice.close();
        }

        mCameraManager = null;
    }

    public void onDestroy() {
        mCameraThread.quitSafely();
        mImageThread.quitSafely();
    }

}
