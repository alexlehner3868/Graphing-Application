# -- Directories --
# Root directory
ROOT ?= .

# Input directories
INCLUDE ?= $(ROOT)/include
SRC ?= $(ROOT)/src


# -- Files --
# Input files
MAIN := $(SRC)/main.c
INCLUDES := $(wildcard $(SRC)/*.h)
SRCS := $(filter-out $(MAIN),$(wildcard $(SRC)/*.c))

# Target files
TARGET ?= $(ROOT)/$(shell basename "$(CURDIR)").c


# -- Targets --
# Combine all files
$(TARGET):
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

# Clean
.PHONY: clean
clean:
	rm -f $(TARGET)
