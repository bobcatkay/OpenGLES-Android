package com.github.opengles_android.examples.image;

import android.os.Bundle;
import android.view.View;
import android.widget.SeekBar;

import com.github.opengles_android.R;
import com.github.opengles_android.common.FullScreenActivity;
import com.github.opengles_android.databinding.ActivityBitmapBinding;

public class ImageActivity extends FullScreenActivity implements SeekBar.OnSeekBarChangeListener {

    private ImageRender mImageRender;
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
        mBinding.seekbarTextureScale.setOnSeekBarChangeListener(this);

        mBinding.glSurfaceView.setEGLContextClientVersion(3);
        mImageRender = new ImageRender(getApplicationContext());
        mBinding.glSurfaceView.setRenderer(mImageRender);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        mImageRender.destroy();
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

            case R.id.seekbar_texture_scale:
                mTransformData.mTextureScale = progress / 100.0f;
                break;
        }

        mImageRender.updateTransform(mTransformData);
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
        mBinding.seekbarTextureScale.setProgress((int) (mTransformData.mTextureScale * mBinding.seekbarScaleY.getProgressUnit()));

    }
}