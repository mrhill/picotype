# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CFLAGS	:=  -DgrUSE_OGR=0					\
		  			-DFIXME=0						\
		  			-DptUSE_FONTWIN=0				\
					-DbbNO_STRICMP=1				\
					-DptUSE_FONTWIN=0				\
					-D ptUSE_FONTWIN=0				\
					-DbbNO_SPRINTF					\
					-DbbCFGENC=0					\

LOCAL_MODULE    := picotype
LOCAL_SRC_FILES := \
					ptFont.cpp	\
					ptFont_mem.cpp	\
					ptFontMan.cpp	\
					ptGC.cpp	\
					ptGC16.cpp	\
					ptGC8.cpp	\
					ptImage.cpp	\
					ptPalMan.cpp	\
					ptSprite.cpp	\

LOCAL_LDLIBS :=	\
				-llog -lc -lgcc -lstdc++ -ldl -lm	\

LOCAL_C_INCLUDES :=	.	\
					$(LOCAL_PATH)/../../babel/include

#LOCAL_STATIC_LIBRARIES := babel

include $(BUILD_STATIC_LIBRARY)
