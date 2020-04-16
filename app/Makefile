ifdef SystemRoot
	SHELL = cmd.exe
	MK_DIR = mkdir
else
	ifeq ($(shell uname), Linux)
		MK_DIR = mkdir -p
	endif

	ifeq ($(shell uname | cut -d _ -f 1), CYGWIN)
		MK_DIR = mkdir -p
	endif

	ifeq ($(shell uname | cut -d _ -f 1), MINGW32)
		MK_DIR = mkdir -p
	endif

	ifeq ($(shell uname | cut -d _ -f 1), MINGW64)
		MK_DIR = mkdir -p
	endif

	ifeq ($(shell uname | cut -d _ -f 1), Darwin)
		MK_DIR = mkdir -p
	endif
endif

SUB_DIRS += \
 \
app

OBJS += \
app/app.o \
app/shell.o \
app/circ_buffer.o \
app/soft_timer.o \
app/event_dispatcher.o \
app/mainloop_timer.o \
app/blinky.o \
app/shellif_usb.o \
app/phy.o \
app/phy_user.o \
app/wpan_ping.o

OBJS_AS_ARGS += \
"app/app.o" \
"app/shell.o" \
"app/circ_buffer.o" \
"app/soft_timer.o" \
"app/event_dispatcher.o" \
"app/mainloop_timer.o" \
"app/blinky.o" \
"app/shellif_usb.o" \
"app/phy.o" \
"app/phy_user.o" \
"app/wpan_ping.o"

DIR_INCLUDES += \
-I"../../app" \
-I"../../"

DEPS_AS_ARGS += \
"app/app.d" \
"app/shell.d" \
"app/circ_buffer.d" \
"app/soft_timer.d" \
"app/event_dispatcher.d" \
"app/mainloop_timer.d" \
"app/blinky.d" \
"app/shellif_usb.d" \
"app/phy.d" \
"app/phy_user.d" \
"app/wpan_ping.d"

vpath %.c ../../
