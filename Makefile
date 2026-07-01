CC ?= gcc
PKG_CONFIG ?= pkg-config

ALLEGRO_PKGS := allegro-5 allegro_main-5 allegro_font-5 allegro_image-5 allegro_primitives-5 allegro_audio-5 allegro_acodec-5

# Debug builds print the selected asset base directory at startup
# remove -DNDEBUG from CFLAGS to enable debug prints when running
CFLAGS ?= -g -Wall -Wextra -DNDEBUG
CPPFLAGS += -Iincludes $(shell $(PKG_CONFIG) --cflags $(ALLEGRO_PKGS))
LDLIBS += $(shell $(PKG_CONFIG) --libs $(ALLEGRO_PKGS))

BUILD_DIR ?= build

ifeq ($(OS),Windows_NT)
    EXE_EXT := .exe
    RUN_PREFIX := .\\
	MKDIR_P = cmd /C if not exist "$(subst /,\\,$1)" mkdir "$(subst /,\\,$1)"
	RM_RF = cmd /C if exist "$(subst /,\\,$1)" rmdir /S /Q "$(subst /,\\,$1)"
else
    EXE_EXT :=
    RUN_PREFIX := ./
	MKDIR_P = mkdir -p $1
	RM_RF = rm -rf $1
endif

TARGET := $(BUILD_DIR)/game$(EXE_EXT)
SRCS := $(wildcard src/*.c) $(wildcard src/entities/*.c)
OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))
SELFTEST_ROUNDS ?= 500

.PHONY: all clean run selftest

all: $(TARGET)

$(TARGET): $(OBJS)
	$(call MKDIR_P,$(dir $@))
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)


$(BUILD_DIR)/%.o: %.c
	$(call MKDIR_P,$(dir $@))
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	$(RUN_PREFIX)$(TARGET)

# example usage: make selftest SELFTEST_ROUNDS=1000 or .\game.exe --self-test 1000
selftest: $(TARGET)
	$(RUN_PREFIX)$(TARGET) --self-test $(SELFTEST_ROUNDS)

clean:
	$(call RM_RF,$(BUILD_DIR))
