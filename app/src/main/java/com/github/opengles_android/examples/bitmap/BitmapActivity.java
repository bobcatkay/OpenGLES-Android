package com.github.opengles_android.examples.bitmap;

import android.os.Bundle;
import android.view.View;
import android.widget.SeekBar;

import com.github.opengles_android.R;
import com.github.opengles_android.common.FullScreenActivity;
import com.github.opengles_android.databinding.ActivityBitmapBinding;

public class BitmapActivity extends FullScreenActivity implements SeekBar.OnSeekBarChangeListener {

    private BitmapRender mBitmapRender;
    private ActivityBitmapBinding mBinding;
    private TransformData mTransformData = new TransformData();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mBinding = ActivityBitmapBinding.inflate(getLayoutInflater());
        setContentView(mBinding.getRoot());

        resetTransform();
        mBinding.seekbarTranslateX.setOnSeekBarChangeListener(this);
        mBinding.seekbarTranslateY.setOnSeekBarChangeListener(this);
        mBinding.seekbarScaleX.setOnSeekBarChangeListener(this);
        mBinding.seekbarScaleY.setOnSeekBarChangeListener(this);
        mBinding.seekbarRotateX.setOnSeekBarChangeListener(this);
        mBinding.seekbarRotateY.setOnSeekBarChangeListener(this);
        mBinding.seekbarRotateZ.setOnSeekBarChangeListener(this);

        mBinding.glSurfaceView.setEGLContextClientVersion(3);
        mBitmapRender = new BitmapRender(getApplicationContext());
        mBinding.glSurfaceView.setRenderer(mBitmapRender);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        mBitmapRender.destroy();
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        switch (seekBar.getId()) {
            case R.id.seekbar_translate_x:
                mTransformData.mTranslateX = progress / 100.0f;
                break;

            case R.id.seekbar_translate_y:
                mTransformData.mTranslateY = progress / 100.0f;
                break;

            case R.id.seekbar_scale_x:
                mTransformData.mScaleX = progress / 100.0f;
                break;

            case R.id.seekbar_scale_y:
                mTransformData.mScaleY = progress / 100.0f;
                break;

            case R.id.seekbar_rotate_x:
                mTransformData.mRotateX = progress;
                break;

            case R.id.seekbar_rotate_y:
                mTransformData.mRotateY = progress;
                break;

            case R.id.seekbar_rotate_z:
                mTransformData.mRotateZ = progress;
                break;
        }

        mBitmapRender.updateTransform(mTransformData);
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {

    }

    public void onClickReset(View view) {
        resetTransform();
    }

    private void resetTransform() {
        mTransformData = new TransformData();
        mBinding.seekbarTranslateX.setProgress((int) (mTransformData.mTranslateX * mBinding.seekbarTranslateX.getProgressUnit()));
        mBinding.seekbarTranslateY.setProgress((int) (mTransformData.mTranslateY * mBinding.seekbarTranslateY.getProgressUnit()));
        mBinding.seekbarRotateX.setProgress((int) (mTransformData.mRotateX * mBinding.seekbarRotateX.getProgressUnit()));
        mBinding.seekbarRotateY.setProgress((int) (mTransformData.mRotateY * mBinding.seekbarRotateY.getProgressUnit()));
        mBinding.seekbarRotateZ.setProgress((int) (mTransformData.mRotateZ * mBinding.seekbarRotateZ.getProgressUnit()));
        mBinding.seekbarScaleX.setProgress((int) (mTransformData.mScaleX * mBinding.seekbarScaleX.getProgressUnit()));
        mBinding.seekbarScaleY.setProgress((int) (mTransformData.mScaleY * mBinding.seekbarScaleY.getProgressUnit()));

    }
}