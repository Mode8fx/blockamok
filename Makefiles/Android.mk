LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := main

LOCAL_SRC_FILES := $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/**/*.c)

$(info LOCAL_PATH is $(LOCAL_PATH))
$(info LOCAL_SRC_FILES is $(LOCAL_SRC_FILES))

LOCAL_CFLAGS += -DANDROID -DNO_QUIT

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog
LOCAL_SHARED_LIBRARIES := SDL2 SDL2_mixer SDL2_ttf

include $(BUILD_SHARED_LIBRARY)

$(call import-module,SDL2)
$(call import-module,SDL2_mixer)
$(call import-module,SDL2_ttf)
