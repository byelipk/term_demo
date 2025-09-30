UNAME_S := $(shell uname -s)

SOEXT = so
LDFLAGS ?= -dynamiclib -undefined dynamic_lookup

# Erlang/OTP headers
ERTS_INCLUDE_DIR ?= ${shell erl -noshell -eval 'io:format("~s/erts-~s/include", [code:root_dir(), erlang:system_info(version)])' -s init stop}

CFLAGS ?= -O2 -Wall -fPIC
CFLAGS += -I$(ERTS_INCLUDE_DIR)
CFLAGS += -Ic_src/vendor
# Feature-test macros recommended by termbox2
CFLAGS += -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=600

ifeq ($(UNAME_S),Darwin)
CFLAGS += -D_DARWIN_C_SOURCE
endif

SRC    = c_src/termbox_nif.c
TARGET = $(MIX_APP_PATH)/priv/termbox_nif.$(SOEXT)

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p $(MIX_APP_PATH)/priv
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $@

clean:
	rm -f $(TARGET)

