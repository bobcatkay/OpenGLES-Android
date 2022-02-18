package com.github.opengles_android.common;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES30;
import android.opengl.GLUtils;
import android.util.Log;

import java.io.InputStream;

import static android.opengl.GLES20.GL_CLAMP_TO_EDGE;
import static android.opengl.GLES20.GL_LINEAR;
import static android.opengl.GLES20.GL_MIRRORED_REPEAT;
import static android.opengl.GLES20.GL_REPEAT;
import static android.opengl.GLES20.GL_RGBA;
import static android.opengl.GLES20.GL_TEXTURE0;
import static android.opengl.GLES20.GL_TEXTURE_2D;
import static android.opengl.GLES20.GL_TEXTURE_MAG_FILTER;
import static android.opengl.GLES20.GL_TEXTURE_MIN_FILTER;
import static android.opengl.GLES20.GL_TEXTURE_WRAP_S;
import static android.opengl.GLES20.GL_TEXTURE_WRAP_T;

public class Texture {
    private final static String TAG = "Texture";

    private int mTexID;
    private int mWidth;
    private int mHeight;

    private Texture(int texID) {
        mTexID = texID;
    }

    public static Texture loadFromBitmap(Bitmap bitmap) {
        int[] id = new int[1];
        GLES30.glGenTextures(1, id, 0);
        GLES30.glBindTexture(GL_TEXTURE_2D, id[0]);

        GLES30.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        GLES30.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//        GLES30.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
//        GLES30.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
//        GLES30.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//        GLES30.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        GLES30.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        GLES30.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        GLUtils.texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap, 0);
        GLES30.glBindTexture(GL_TEXTURE_2D, 0);

        Texture texture = new Texture(id[0]);
        texture.mWidth = bitmap.getWidth();
        texture.mHeight = bitmap.getHeight();
        int error = Utils.checkGLError();

        if (0 == error) {
            Log.d(TAG, "loadFromBitmap, width: " + texture.mWidth + ", height: " + texture.mHeight);
        }

        return texture;
    }

    public static Texture loadFromAssets(Context context, String fileName) {
        try {
            InputStream inputStream = context.getResources().getAssets().open(fileName);
            Bitmap bitmap = BitmapFactory.decodeStream(inputStream);
            Texture texture = loadFromBitmap(bitmap);
            inputStream.close();

            return texture;
        } catch (Exception e) {
            e.printStackTrace();
        }

        return null;
    }

    public void bind(int texUnit) {
        GLES30.glActiveTexture(GL_TEXTURE0 + texUnit);
        GLES30.glBindTexture(GL_TEXTURE_2D, mTexID);
    }

    public int getWidth() {
        return mWidth;
    }

    public int getHeight() {
        return mHeight;
    }

    public void release() {
        GLES30.glDeleteTextures(1, new int[] {mTexID}, 0);
    }
}
