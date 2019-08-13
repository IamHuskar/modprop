LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := modprop
LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_SRC_FILES := su/a.c
include $(BUILD_EXECUTABLE)
