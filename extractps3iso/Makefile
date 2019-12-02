CC			:=	$(PREFIX)gcc
CXX			:=	$(PREFIX)g++
AR			:=	$(PREFIX)ar
OBJCOPY		:=	$(PREFIX)objcopy

UNAME		:=	$(shell uname -s)

CFLAGS		:=	$(DEBUGFLAGS) -O2
CFLAGS		+=	$(INCLUDES)
CXXFLAGS	:=	$(CFLAGS) -fno-rtti -fno-exceptions
LDFLAGS		=	$(DEBUGFLAGS)

ifeq ($(strip $(PS3DEV)),)
  ifeq ($(strip $(DEVKITPS3)),)
    export PS3DEV := /usr/local/ps3dev
  else
    export PS3DEV := $(DEVKITPS3)
  endif
endif

ifneq (,$(findstring MINGW,$(UNAME)))
	PLATFORM	:= win32
	POSTFIX		:= .exe
	CFLAGS		+= -I/include -mno-cygwin
	LDFLAGS		+= -L/lib -mno-cygwin -s -Wall
	OS			:= win32
endif

ifneq (,$(findstring CYGWIN,$(UNAME)))
	LDFLAGS		+= -s 
	POSTFIX		:= .exe
	OS			:= win32
endif

ifneq (,$(findstring Darwin,$(UNAME)))
	SDK			:= /Developer/SDKs/MacOSX10.4u.sdk 
	OSXCFLAGS	:= -mmacosx-version-min=10.4 -arch i386
	OSXCXXFLAGS	:=	$(OSXCFLAGS)
	CXXFLAGS	+=	-fvisibility=hidden
	LDFLAGS		+= -mmacosx-version-min=10.4 -arch i386 -Wl,-syslibroot,$(SDK) -L/opt/local/lib
endif

ifneq (,$(findstring BSD,$(UNAME)))
	CFLAGS += -I/usr/local/include
	LDFLAGS += -L/usr/local/lib 
endif

ifneq (,$(findstring SunOS,$(UNAME)))
        CFLAGS += -I/opt/csw/include
        LDFLAGS += -L/opt/csw/lib -R/opt/csw/lib 
endif

ifneq (,$(findstring Linux,$(UNAME)))
	LDFLAGS += 
	OS		:= Linux
endif

TARGETS	:=	extractps3iso$(POSTFIX)

all: $(TARGETS)

extractps3iso$(POSTFIX):extractps3iso.c
	@echo building ... $(notdir $@)
	@$(CC) $< $(CFLAGS) $(LDFLAGS) -o $@

clean:
	@echo clean ...
	@rm -f $(TARGETS)

.PHONY: clean
