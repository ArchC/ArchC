# Use: make TARGET=<target architecture>
# It looks for the following commands for this target:
CC       = $(TARGET)-gcc
AR       = $(TARGET)-ar
RANLIB   = $(TARGET)-ranlib

# Flags
CFLAGS   = -O2 -g -I. -I..
ARFLAGS  = cr
RM       = rm
RMFLAGS  = -rf

# Targets
LIBSYSC = libac_sysc.a

# Sources
LIBSYSC_SOURCES = $(wildcard *.c)

# Object files
LIBSYSC_OBJECTS = $(patsubst %.c,%.o,$(notdir $(LIBSYSC_SOURCES)))

# Target rules

all: $(LIBSYSC)

%.o: $(LIBSYSC_DIR)/%.c

$(LIBSYSC): $(LIBSYSC_OBJECTS)
	$(AR) $(ARFLAGS) $(LIBSYSC) $(LIBSYSC_OBJECTS)
	$(RANLIB) $(LIBSYSC)
	-$(RM) $(RMFLAGS) $(LIBSYSC_OBJECTS)

# Clean
clean:
	-$(RM) $(RMFLAGS) $(LIBSYSC) $(LIBSYSC_OBJECTS)
