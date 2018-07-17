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
HEADEREXT := h
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))

# Folder Lists
INCDIRS := $(shell find $(SRCDIR)/**/* -name '*.$(SRCEXT)' -exec dirname {} \; | sort | uniq)
INCLIST := $(patsubst $(SRCDIR)/%,-I $(SRCDIR)/%,$(INCDIRS))
BUILDLIST := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(INCDIRS))

FFMPEG_PATH=$(HOME)/ffmpeg
FFMPEG_CONFIG_OPTS += --enable-gpl
FFMPEG_CONFIG_OPTS += --enable-shared --disable-static
FFMPEG_CONFIG_OPTS += --disable-doc
FFMPEG_CONFIG_OPTS += --enable-postproc

PACKAGES += libvpx-dev libopus-dev libx264-dev libx265-dev libasound2-dev yasm

# Shared Compiler Flags
CFLAGS := -std=c++11 -O3 -Wall -Wextra
INC := -I include $(INCLIST) -I /usr/local/include
LIB := -lasound -pthread `pkg-config --libs libavformat` `pkg-config --libs libavdevice` `pkg-config --libs libavcodec` `pkg-config --libs libavutil` `pkg-config --libs libswscale` `pkg-config --libs libswresample`  -lm -lrt

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
	@echo "${Y}Compiling${N} ${B}$< ${N}"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

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

install-ffmpeg: install-dependencies
	@ cd ${FFMPEG_PATH} && ./configure ${FFMPEG_CONFIG_OPTS} && make -j${MAKE_J} && sudo make install

install-dependencies:
	dpkg-query -W ${PACKAGES} || sudo apt-get install ${PACKAGES}

clone:
	git clone --branch n4.0 https://git.ffmpeg.org/ffmpeg.git ${FFMPEG_PATH}

run:
	${TARGET}

.PHONY: clean cleantests test
