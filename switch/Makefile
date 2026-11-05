#---------------------------------------------------------------------------------
# Mickey & Donald Collection - Nintendo Switch homebrew build (devkitPro / libnx).
#
#   cd switch && make            # -> MDC.nro
#   make NXLINK=1                # also logs over nxlink (printf / stderr / a 5 s fps line), see README.md
#   make clean
#
# Needs the devkitPro Switch toolchain and these portlibs (dkp-pacman -S switch-dev switch-sdl2 switch-mesa
# switch-freetype switch-harfbuzz switch-libpng switch-bzip2 switch-zlib). Based on libnx's template Makefile, with an explicit
# source list: the emulation core, the game modules and the app are shared with the Windows and Android builds;
# the video / audio back ends are the Android build's (OpenGL ES 3 + SDL2), the rest is src/platform/switch.
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITPRO)/libnx/switch_rules

#---------------------------------------------------------------------------------
TARGET      := MickeyDonald_nx
BUILD       := build
SRC         := ../src
SOURCES     := $(SRC)/core $(SRC)/game $(SRC)/app $(SRC)/platform/android $(SRC)/platform/switch
INCLUDES    := $(SRC)

# (file names, searched for in SOURCES; they are unique across the folders)
CPPFILES    := m68k.cpp z80.cpp vdp.cpp ym2612.cpp psg.cpp md.cpp \
               common.cpp coi.cpp qs.cpp woi.cpp \
               app.cpp app_switch.cpp app_achievements.cpp achievements.cpp app_menus.cpp launcher.cpp \
               jukebox.cpp session.cpp soundtrack.cpp settings.cpp palette.cpp audio_mixer.cpp romfind.cpp \
               unzip.cpp sys_switch.cpp png.cpp rewind.cpp ui.cpp \
               video_gles.cpp audio_sdl.cpp \
               nx_platform.cpp input_switch.cpp glyphs_ft.cpp

APP_TITLE   := Mickey & Donald Collection
APP_AUTHOR  := simplePsyduck & Darhkfox
APP_VERSION := 1.0
ICON        := icon.jpg

#---------------------------------------------------------------------------------
ARCH     := -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE
DEFINES  := -D__SWITCH__ -DMDC_SWITCH
ifeq ($(NXLINK),1)
DEFINES  += -DMDC_NXLINK
endif

# (the emulation core needs optimising to run at full speed)
CFLAGS   := -g -Wall -O2 -ffunction-sections $(ARCH) $(DEFINES) -Wno-unused-parameter -Wno-sign-compare
CFLAGS   += $(INCLUDE)
CXXFLAGS := $(CFLAGS) -std=gnu++17
ASFLAGS  := -g $(ARCH)
LDFLAGS   = -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

# SDL2 (video through Mesa's EGL / OpenGL ES, audio), FreeType for the UI text, libnx
# (devkitPro's FreeType is built with HarfBuzz, so it needs -lharfbuzz; the group lets the two reference each other)
LIBS     := -lSDL2 -lGLESv2 -lEGL -lglapi -ldrm_nouveau \
            -Wl,--start-group -lfreetype -lharfbuzz -Wl,--end-group -lpng -lbz2 -lz -lnx -lm
LIBDIRS  := $(PORTLIBS) $(LIBNX)

#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT  := $(CURDIR)/$(TARGET)
export TOPDIR  := $(CURDIR)
export VPATH   := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)

export LD      := $(CXX)
export OFILES  := $(CPPFILES:.cpp=.o)

export INCLUDE := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                  $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                  $(foreach dir,$(LIBDIRS),-I$(dir)/include/SDL2 -I$(dir)/include/freetype2) \
                  -I$(CURDIR)/$(BUILD)
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

export APP_ICON := $(TOPDIR)/$(ICON)
export NROFLAGS += --icon=$(APP_ICON)
export NROFLAGS += --nacp=$(CURDIR)/$(TARGET).nacp

.PHONY: $(BUILD) clean all

all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).nro $(TARGET).nacp $(TARGET).elf

#---------------------------------------------------------------------------------
else
.PHONY: all

DEPENDS := $(OFILES:.o=.d)

all : $(OUTPUT).nro

$(OUTPUT).nro : $(OUTPUT).elf $(OUTPUT).nacp

$(OUTPUT).elf : $(OFILES)

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
