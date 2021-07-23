package com.github.opengles_android.common;

import android.content.Context;
import android.opengl.GLES30;
import android.util.Log;

import java.util.HashMap;
import java.util.Map;

public class Shader {
    private final static String TAG = "Shader";
    private int mProgram;
    private Map<String, Object> mParamLocations = new HashMap<>();

    public Shader(Context context, String vertexFileName, String fragmentFileName) {
        this(Utils.getStringFromAssets(context, vertexFileName), Utils.getStringFromAssets(context, fragmentFileName));
    }

    public Shader(String vertexCode, String fragCode) {
        int vertexShader = loadShader(vertexCode, GLES30.GL_VERTEX_SHADER);
        int fragShader = loadShader(fragCode, GLES30.GL_FRAGMENT_SHADER);
        mProgram = createProgram(vertexShader, fragShader);
    }

    private int createProgram(int vShader, int fShader) {
        int program = GLES30.glCreateProgram();

        if (0 == program) {
            Log.e(TAG, "createProgram: Create program error!", new Throwable());
            return 0;
        }

        GLES30.glAttachShader(program, vShader);
        GLES30.glAttachShader(program, fShader);

        GLES30.glLinkProgram(program);

        int[] result = new int[1];
        GLES30.glGetProgramiv(program, GLES30.GL_LINK_STATUS, result, 0);

        if (0 == result[0]) {
            String log = GLES30.glGetProgramInfoLog(program);
            Log.e(TAG, "loadShader: Link program error!" + log, new Throwable());
            return 0;
        }

        return program;
    }

    private int loadShader(String code, int type) {
        int shader = GLES30.glCreateShader(type);
        GLES30.glShaderSource(shader, code);
        GLES30.glCompileShader(shader);

        int[] result = new int[1];
        GLES30.glGetShaderiv(shader, GLES30.GL_COMPILE_STATUS, result, 0);

        if (0 == result[0]) {
            String log = GLES30.glGetShaderInfoLog(shader);
            Log.e(TAG, "loadShader: Compile shader error!\n"
                    + "code: " + code + "\n"
                    + log, new Throwable());
            return 0;
        }

        return shader;
    }

    public void setInt(String name, int value) {
        GLES30.glUniform1i(getUniformLocation(name), value);
    }

    public void setMat4(String name, float[] value) {
        GLES30.glUniform4fv(getUniformLocation(name), 1, value, 0);
    }

    private int getUniformLocation(String name) {
        int location;

        if (null == mParamLocations.get(name)) {
            location = GLES30.glGetUniformLocation(mProgram, name);
            Log.i(TAG, "getUniformLocation, Set location, name: " + name + ", location: " + location);
            mParamLocations.put(name, location);
        } else {
            location = (int) mParamLocations.get(name);
        }

        return location;
    }

    public void use() {
        GLES30.glUseProgram(mProgram);
    }
}
