package com.github.opengles_android.common;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Point;
import android.opengl.GLES30;
import android.provider.Settings;
import android.util.Log;
import android.util.Size;
import android.view.WindowManager;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

public class Utils {
    private final static String TAG = "Utils";

    public final static int FLOAT_SIZE = Float.SIZE / Byte.SIZE;

    public static String getStringFromAssets(Context context, String fileName){
        try {
            InputStreamReader inputReader = new InputStreamReader(context.getResources().getAssets().open(fileName));
            BufferedReader bufReader = new BufferedReader(inputReader);
            String line;
            StringBuilder result = new StringBuilder();

            while ((line = bufReader.readLine()) != null) {
                result.append(line);
                result.append("\n");
            }

            return result.toString();
        } catch (Exception e) {
            e.printStackTrace();
        }

        return "";
    }

    public static FloatBuffer floatArrayToBuffer(float[] arr) {
        ByteBuffer byteBuffer = ByteBuffer.allocateDirect(arr.length * FLOAT_SIZE);
        byteBuffer.order(ByteOrder.nativeOrder());
        FloatBuffer floatBuffer = byteBuffer.asFloatBuffer();
        floatBuffer.put(arr);
        floatBuffer.position(0);
        return floatBuffer;
    }

    public static FloatBuffer bitmapToBuffer(Bitmap bitmap) {
        ByteBuffer byteBuffer = ByteBuffer.allocateDirect(4 * bitmap.getWidth() * bitmap.getHeight());
        byteBuffer.order(ByteOrder.nativeOrder());
        bitmap.copyPixelsToBuffer(byteBuffer);
        return byteBuffer.asFloatBuffer();
    }

    public static Size getScreenResolution(Context context) {
        WindowManager wm = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        Point point = new Point();
        wm.getDefaultDisplay().getRealSize(point);

        Log.d(TAG, "getScreenResolution, width: " + point.x + ", height: " + point.y);

        return new Size(point.x, point.y);
    }

    public static float[] genQuadVertieces() {
        float vertices[] = {
                // positions       // texCoords
                -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
                1.0f, -1.0f, 0.0f,  1.0f, 0.0f,

                -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
                1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
                1.0f,  1.0f, 0.0f,  1.0f, 1.0f
        };

        return vertices;
    }

    public static int getNavigationBarHeight(Context context) {
        int navBarHeight = 0;
        int navGestureValue = 0;

        try {
            navGestureValue = Settings.Secure.getInt(context.getContentResolver(), "navigation_gesture");
        } catch (Settings.SettingNotFoundException e) {
            e.printStackTrace();
        }

        if (0 == navGestureValue) {
            int identifier = context.getResources().getIdentifier("navigation_bar_height", "dimen", "android");

            if (identifier > 0) {
                navBarHeight = context.getResources().getDimensionPixelSize(identifier);
            }
        }

        return navBarHeight;
    }

    public static int getStatusBarHeight(Context context) {
        int statusHeight = 0;
        int identifier = context.getResources().getIdentifier("status_bar_height", "dimen", "android");

        if (identifier > 0) {
            statusHeight = context.getResources().getDimensionPixelSize(identifier);
        }

        return statusHeight;
    }

    public static int checkGLError() {
        int error = GLES30.glGetError();

        if (0 != error) {
            Log.e(TAG, "checkGLError, error: " + error + "\n" + Log.getStackTraceString(new Throwable()));
        }

        return error;
    }
}
