-include colors.mk

CC = g++

# Folders
SRCDIR := src
BUILDDIR := build
TARGETDIR := bin
TESTBUILDDIR := build_tests

# Targets
EXECUTABLE := fogo-player
TARGET := $(TARGETDIR)/$(EXECUTABLE)

# Final Paths
INSTALLBINDIR := /usr/local/bin

# Code Lists
SRCEXT := cpp
HEADEREXT := hpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))

# Folder Lists
INCDIRS := $(shell find $(SRCDIR)/**/* -name '*.$(SRCEXT)' -exec dirname {} \; | sort | uniq)
INCLIST := $(patsubst $(SRCDIR)/%,-I $(SRCDIR)/%,$(INCDIRS))
BUILDLIST := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(INCDIRS))

FFMPEG_PATH=$(HOME)/ffmpeg
FFMPEG_CONFIG_OPTS += --enable-shared --disable-static --enable-gpl --enable-nonfree --enable-pthreads
#FFMPEG_CONFIG_OPTS += --enable-shared# --disable-static --enable-avfilter --enable-swscale --enable-postproc
FFMPEG_CONFIG_OPTS += --enable-libx264 --enable-libx265 --enable-libvpx #--enable-libjack
#FFMPEG_CONFIG_OPTS += --enable-muxer=h264 --enable-demuxer=h264 --enable-decoder=h264
#FFMPEG_CONFIG_OPTS += --enable-muxer=hevc --enable-demuxer=hevc --enable-decoder=hevc
#FFMPEG_CONFIG_OPTS += --enable-libvdpau --enable-libva # Harwdware acceleration
#FFMPEG_CONFIG_OPTS += --enable-decoder=vp8
#FFMPEG_CONFIG_OPTS += --enable-decoder=vp9
#FFMPEG_CONFIG_OPTS += --enable-muxer=webm
FFMPEG_CONFIG_OPTS += --disable-doc --enable-pthreads --enable-filters
#FFMPEG_CONFIG_OPTS += --enable-postproc

OPENCV_PATH=$(HOME)/opencv

OPENCV_PACKAGES += cmake git libgtk2.0-dev pkg-config libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev libv4l-dev
FFMPEG_PACKAGES += libvpx-dev libopus-dev libx264-dev libx265-dev libasound2-dev libvdpau-dev libva-dev yasm
SDL_PACKAGES += libsdl2-dev libsdl2-mixer-dev
ZBAR_PACKAGES += libzbar0 libzbar-dev
POINT_GREY_PACKAGES += #libraw1394-11  libgtkmm-2.4-dev libglademm-2.4-dev libgtkglextmm-x11-1.2-dev libusb-1.0-0 #libavcodec-ffmpeg56 libavformat-ffmpeg56 libswscale-ffmpeg3 libswresample-ffmpeg1 libavutil-ffmpeg54

PACKAGES += $(OPENCV_PACKAGES) $(FFMPEG_PACKAGES) $(ZBAR_PACKAGES) $(SDL_PACKAGES) $(POINT_GREY_PACKAGES)
# Shared Compiler Flags
CFLAGS := -std=c++14 -O3 -Wall -Wextra
INC := -I include $(INCLIST) -I /usr/local/include

SDL_LIB    += `pkg-config --libs sdl2` `pkg-config --libs SDL2_mixer`
FFMPEG_LIB += `pkg-config --libs libavformat` `pkg-config --libs libavdevice` `pkg-config --libs libavcodec` `pkg-config --libs libavutil` `pkg-config --libs libswscale` `pkg-config --libs libswresample` 
OPENCV_LIB += `pkg-config --libs opencv`
ALSA_LIB += -lasound 
ZBAR_LIB += -lzbar

LIB := -pthread -lm -lrt  $(FFMPEG_LIB) $(OPENCV_LIB) $(ALSA_LIB) $(ZBAR_LIB) $(SDL_LIB)

ifeq ($(debug), 1)
CFLAGS += -g -ggdb3 -D DEBUG
else
CFLAGS += -DNDEBUG
endif

$(TARGET): $(OBJECTS)
	@mkdir -p $(TARGETDIR)
	@echo  "${P}Linking all targets...${N}"
	@$(CC) $^ -o $(TARGET) $(LIB)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDLIST)
	@echo "CC    $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

test: $(TARGET)
	@$(MAKE) --no-print-directory -C test
	
memtest: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all --log-file=valgrind-out.txt $(TARGET)

clean: cleantests
	@echo "${R}Cleaning $(TARGET)${N}"; $(RM) -r $(BUILDDIR) $(TARGETDIR)

cleantests:
	@echo "${R}Cleaning tests${N}"; $(RM) -r $(TESTBUILDDIR)
	
install:
	@echo "${Y}Installing ${N} ${B} $(EXECUTABLE) ${N}"; cp $(TARGET) $(INSTALLBINDIR)

distclean:
	@echo "${R}Removing ${N} ${B} $(EXECUTABLE) ${N}"; rm $(INSTALLBINDIR)/$(EXECUTABLE)

install-ffmpeg: install-dependencies clone-ffmpeg
	@ cd ${FFMPEG_PATH} && ./configure ${FFMPEG_CONFIG_OPTS}  && make -j4 && sudo make install && sudo ldconfig

install-opencv: install-dependencies clone-opencv
	@ cd ${OPENCV_PATH} && mkdir -p build  && cd build && cmake -DWITH_LIBV4L=ON ../ -DCMAKE_BUILD_TYPE=Release && make -j4 && sudo make install && sudo ldconfig

install-dependencies:
	dpkg-query -W ${PACKAGES} || sudo apt-get install ${PACKAGES}

clone-ffmpeg:
	git clone --branch n4.0 https://git.ffmpeg.org/ffmpeg.git ${FFMPEG_PATH}
	git clone --branch 3.4 https://github.com/opencv/opencv.git ${OPENCV_PATH}	

clone-opencv:
	git clone --branch 3.4.2 https://github.com/opencv/opencv ${OPENCV_PATH}

run:
	${TARGET}

.PHONY: clean cleantests test
