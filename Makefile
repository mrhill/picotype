PROJECT  := picotype
TARGET   := $(PROJECT).a
TOPDIR   := $(PWD)/..
INCDIR   := $(TOPDIR)/babel/include
SRCDIR   := $(PWD)/src
CXXFLAGS := -DbbSIZEOF_INT=4 -DbbSIZEOF_UPTR=8 -DptUSE_FONTWIN=0
include $(TOPDIR)/babel/common.mk
