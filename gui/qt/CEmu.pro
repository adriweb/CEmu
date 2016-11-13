# Warn if Qt isn't as updated as it should be
lessThan(QT_MAJOR_VERSION, 5) : error("You need at least Qt 5.6 to build CEmu!")
lessThan(QT_MINOR_VERSION, 6) : error("You need at least Qt 5.6 to build CEmu!")

# Warn if git submodules not downloaded
!exists("lua/lua/src/lua.h"): error("You have to run 'git submodule init' and 'git submodule update' first.")

# CEmu version
if (0) { # GitHub release/deployment build. Has to correspond to the git tag.
    DEFINES += CEMU_VERSION=\\\"1.0\\\"
} else { # Development build. Used in the about screen
    GIT_VERSION = $$system(git describe --abbrev=7 --dirty --always --tags)
    DEFINES += CEMU_VERSION=\\\"0.9dev_$$GIT_VERSION\\\"
}

# Continuous Integration (variable checked later)
CI = $$(CI)

# Code beautifying
DISTFILES += ../../.astylerc

QT += core gui widgets network

TARGET = CEmu
TEMPLATE = app

# Localization
TRANSLATIONS += i18n/fr_FR.ts i18n/es_ES.ts i18n/nl_NL.ts

# We use C++11, but Sol also uses C++14.
CONFIG += c++14 console

# Core options
DEFINES += DEBUG_SUPPORT

CONFIG(release, debug|release) {
    #This is a release build
    DEFINES += QT_NO_DEBUG_OUTPUT
} else {
    #This is a debug build
    GLOBAL_FLAGS += -g3
}

# TODO Lua: adjust options by platforms, see Makefile
GLOBAL_FLAGS += -DLUA_USE_LONGJMP

# GCC/clang flags
if (!win32-msvc*) {
    GLOBAL_FLAGS    += -W -Wall -Wextra -Wunused-function -Werror=write-strings -Werror=redundant-decls -Werror=format -Werror=format-security -Werror=declaration-after-statement -Werror=implicit-function-declaration -Werror=date-time -Werror=missing-prototypes -Werror=return-type -Werror=pointer-arith -Winit-self
    GLOBAL_FLAGS    += -ffunction-sections -fdata-sections -fno-strict-overflow
    QMAKE_CFLAGS    += -std=gnu11
    QMAKE_CXXFLAGS  += -fno-exceptions
    isEmpty(CI) {
        # Only enable opts for non-CI release builds
        # -flto might cause an internal compiler error on GCC in some circumstances (with -g3?)... Comment it if needed.
        CONFIG(release, debug|release): GLOBAL_FLAGS += -O3 -flto
    }
} else {
    # TODO: add equivalent flags
    # Example for -Werror=shadow: /weC4456 /weC4457 /weC4458 /weC4459
    #     Source: https://connect.microsoft.com/VisualStudio/feedback/details/1355600/
    QMAKE_CXXFLAGS  += /Wall
}

if (macx|linux) {
    # Be more secure by default...
    GLOBAL_FLAGS    += -fPIE -Wstack-protector -fstack-protector-strong --param=ssp-buffer-size=1
    # Use ASAN on debug builds. Watch out about ODR crashes when built with -flto. detect_odr_violation=0 as an env var may help.
    CONFIG(debug, debug|release): GLOBAL_FLAGS += -fsanitize=address,bounds -fsanitize-undefined-trap-on-error -O0
}

macx:  QMAKE_LFLAGS += -Wl,-dead_strip
linux: QMAKE_LFLAGS += -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -Wl,--gc-sections -pie

QMAKE_CFLAGS    += $$GLOBAL_FLAGS
QMAKE_CXXFLAGS  += $$GLOBAL_FLAGS
QMAKE_LFLAGS    += $$GLOBAL_FLAGS

macx: ICON = resources/icons/icon.icns

SOURCES +=  utils.cpp \
    main.cpp \
    mainwindow.cpp \
    romselection.cpp \
    qtframebuffer.cpp \
    lcdwidget.cpp \
    emuthread.cpp \
    datawidget.cpp \
    dockwidget.cpp \
    searchwidget.cpp \
    basiccodeviewerwindow.cpp \
    sendinghandler.cpp \
    debugger.cpp \
    hexeditor.cpp \
    settings.cpp \
    ipc.cpp \
    keyhistory.cpp \
    memoryvisualizer.cpp \
    luascripting.cpp \
    luaeditor.cpp \
    keypad/qtkeypadbridge.cpp \
    keypad/keymap.cpp \
    keypad/keypadwidget.cpp \
    keypad/rectkey.cpp \
    keypad/arrowkey.cpp \
    qhexedit/chunks.cpp \
    qhexedit/commands.cpp \
    qhexedit/qhexedit.cpp \
    capture/gif.cpp \
    capture/optimize.c \
    capture/opttemplate.c \
    capture/gifread.c \
    capture/gifwrite.c \
    capture/quantize.c \
    capture/giffunc.c \
    tivarslib/utils_tivarslib.cpp \
    tivarslib/TypeHandlers/DummyHandler.cpp \
    tivarslib/TypeHandlers/TH_0x00.cpp \
    tivarslib/TypeHandlers/TH_0x01.cpp \
    tivarslib/TypeHandlers/TH_0x02.cpp \
    tivarslib/TypeHandlers/TH_0x05.cpp \
    tivarslib/TypeHandlers/TH_0x0C.cpp \
    tivarslib/TypeHandlers/TH_0x0D.cpp \
    tivarslib/TypeHandlers/TH_0x1B.cpp \
    tivarslib/TypeHandlers/TH_0x1C.cpp \
    tivarslib/TypeHandlers/TH_0x1D.cpp \
    tivarslib/TypeHandlers/TH_0x1E.cpp \
    tivarslib/TypeHandlers/TH_0x1F.cpp \
    tivarslib/TypeHandlers/TH_0x20.cpp \
    tivarslib/TypeHandlers/TH_0x21.cpp \
    ../../tests/autotester/autotester.cpp \
    lua/lua/src/lapi.c \
    lua/lua/src/lauxlib.c \
    lua/lua/src/lbaselib.c \
    lua/lua/src/lbitlib.c \
    lua/lua/src/lcode.c \
    lua/lua/src/lcorolib.c \
    lua/lua/src/lctype.c \
    lua/lua/src/ldblib.c \
    lua/lua/src/ldebug.c \
    lua/lua/src/ldo.c \
    lua/lua/src/ldump.c \
    lua/lua/src/lfunc.c \
    lua/lua/src/lgc.c \
    lua/lua/src/linit.c \
    lua/lua/src/liolib.c \
    lua/lua/src/llex.c \
    lua/lua/src/lmathlib.c \
    lua/lua/src/lmem.c \
    lua/lua/src/loadlib.c \
    lua/lua/src/lobject.c \
    lua/lua/src/lopcodes.c \
    lua/lua/src/loslib.c \
    lua/lua/src/lparser.c \
    lua/lua/src/lstate.c \
    lua/lua/src/lstring.c \
    lua/lua/src/lstrlib.c \
    lua/lua/src/ltable.c \
    lua/lua/src/ltablib.c \
    lua/lua/src/ltm.c \
    lua/lua/src/lundump.c \
    lua/lua/src/lutf8lib.c \
    lua/lua/src/lvm.c \
    lua/lua/src/lzio.c \
    ../../core/asic.c \
    ../../core/cpu.c \
    ../../core/keypad.c \
    ../../core/lcd.c \
    ../../core/registers.c \
    ../../core/port.c \
    ../../core/interrupt.c \
    ../../core/flash.c \
    ../../core/misc.c \
    ../../core/schedule.c \
    ../../core/timers.c \
    ../../core/usb.c \
    ../../core/sha256.c \
    ../../core/realclock.c \
    ../../core/backlight.c \
    ../../core/cert.c \
    ../../core/control.c \
    ../../core/mem.c \
    ../../core/link.c \
    ../../core/vat.c \
    ../../core/emu.c \
    ../../core/extras.c \
    ../../core/debug/disasm.cpp \
    ../../core/debug/debug.cpp \
    ../../core/debug/stepping.cpp

linux|macx: SOURCES += ../../core/os/os-linux.c
win32: SOURCES += ../../core/os/os-win32.c win32-console.cpp
win32: LIBS += -lpsapi

HEADERS  +=  utils.h \
    mainwindow.h \
    romselection.h \
    qtframebuffer.h \
    lcdwidget.h \
    emuthread.h \
    datawidget.h \
    dockwidget.h \
    searchwidget.h \
    cemuopts.h \
    sendinghandler.h \
    debugger.h \
    ipc.h \
    keyhistory.h \
    memoryvisualizer.h \
    basiccodeviewerwindow.h \
    keypad/qtkeypadbridge.h \
    keypad/keymap.h \
    keypad/keypadwidget.h \
    keypad/key.h \
    keypad/keyconfig.h \
    keypad/rectkey.h \
    keypad/graphkey.h \
    keypad/secondkey.h \
    keypad/alphakey.h \
    keypad/otherkey.h \
    keypad/numkey.h \
    keypad/operkey.h \
    keypad/arrowkey.h \
    keypad/keycode.h \
    qhexedit/chunks.h \
    qhexedit/commands.h \
    qhexedit/qhexedit.h \
    capture/gif.h \
    capture/giflib.h \
    capture/kcolor.h \
    capture/gifsicle.h \
    capture/lcdfgif/gif.h \
    tivarslib/autoloader.h \
    tivarslib/utils_tivarslib.h \
    tivarslib/TypeHandlers/TypeHandlerFuncGetter.h \
    tivarslib/TypeHandlers/TypeHandlers.h \
    ../../tests/autotester/autotester.h \
    lua/lua/src/lapi.h \
    lua/lua/src/lauxlib.h \
    lua/lua/src/lcode.h \
    lua/lua/src/lctype.h \
    lua/lua/src/ldebug.h \
    lua/lua/src/ldo.h \
    lua/lua/src/lfunc.h \
    lua/lua/src/lgc.h \
    lua/lua/src/llex.h \
    lua/lua/src/llimits.h \
    lua/lua/src/lmem.h \
    lua/lua/src/lobject.h \
    lua/lua/src/lopcodes.h \
    lua/lua/src/lparser.h \
    lua/lua/src/lprefix.h \
    lua/lua/src/lstate.h \
    lua/lua/src/lstring.h \
    lua/lua/src/ltable.h \
    lua/lua/src/ltm.h \
    lua/lua/src/lua.h \
    lua/lua/src/luaconf.h \
    lua/lua/src/lualib.h \
    lua/lua/src/lundump.h \
    lua/lua/src/lvm.h \
    lua/lua/src/lzio.h \
    lua/lua/src/lua.hpp \
    lua/sol.hpp \
    ../../core/asic.h \
    ../../core/cpu.h \
    ../../core/defines.h \
    ../../core/keypad.h \
    ../../core/lcd.h \
    ../../core/registers.h \
    ../../core/tidevices.h \
    ../../core/port.h \
    ../../core/interrupt.h \
    ../../core/emu.h \
    ../../core/flash.h \
    ../../core/misc.h \
    ../../core/schedule.h \
    ../../core/timers.h \
    ../../core/usb.h \
    ../../core/sha256.h \
    ../../core/realclock.h \
    ../../core/backlight.h \
    ../../core/cert.h \
    ../../core/control.h \
    ../../core/mem.h \
    ../../core/link.h \
    ../../core/vat.h \
    ../../core/extras.h \
    ../../core/os/os.h \
    ../../core/debug/debug.h \
    ../../core/debug/disasm.h \
    ../../core/debug/stepping.h

FORMS    += mainwindow.ui \
    romselection.ui \
    searchwidget.ui \
    basiccodeviewerwindow.ui \
    keyhistory.ui \
    memoryvisualizer.ui

RESOURCES += \
    resources.qrc

RC_ICONS += resources\icons\icon.ico
