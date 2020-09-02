# Root directory
ROOT ?= .
# Input directories
INCLUDE ?= $(ROOT)/include
SRC ?= $(ROOT)/src
# Build directories
BUILD ?= $(ROOT)/build

# Prerequisites
MAIN := $(SRC)/main.c
SRCS := $(filter-out $(MAIN),$(wildcard $(SRC)/*.c))
INCLUDES := $(wildcard $(SRC)/*.h)
# Targets
TARGET ?= $(BUILD)/$(shell basename "$(CURDIR)").c

# Commands
MKDIR = mkdir -p
RM = rm -rfv

# Build target file
.PHONY: all
all: $(TARGET)

$(TARGET):
	@$(MKDIR) $(BUILD)
	cat $(INCLUDES) $(SRCS) $(MAIN) | grep '#include <.*>' | uniq | sort >| $(TARGET)
	@echo >> $(TARGET)
	cat $(INCLUDES) $(SRCS) $(MAIN) | grep '#define' >> $(TARGET)
	@echo >> $(TARGET)
ifneq ($(INCLUDES),)
	cat $(INCLUDES) | grep -vE '#include|#define' | >> $(TARGET)
endif
ifneq ($(SRCS),)
	cat $(SRCS) | grep -vE '#include|#define' | >> $(TARGET)
endif
	cat $(MAIN) | grep -vE '#include|#define' >> $(TARGET)
	@command -v clang-format &> /dev/null && clang-format -i $(TARGET)

# Clean build directory
.PHONY: clean
clean:
	@$(RM) $(BUILD)
