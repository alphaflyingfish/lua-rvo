LUALIB_SRC = .
CC = g++

LUA_CLIB_DIR = ../../lib/luaclib
LUA_STATICLIB := ../../skynet/3rd/lua/liblua.a
LUA_INC ?= ../../skynet/3rd/lua

uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
ifeq ($(uname_S), Darwin)
	CFLAGS = -g -O2 -Wall -I$(LUA_INC)
	SHARED = -fPIC
	RVO_CFLAGS = -bundle -undefined dynamic_lookup	
else
	CFLAGS = -g -O2 -Wall -I$(LUA_INC)
	SHARED := -fPIC --shared
endif

# CFLAGS = -g -O2 -Wall -I$(LUA_INC)
# $(CC) $(SHARED) $(CFLAGS) $^ -Wl,-rpath,../3rd/rvo -lrvo -o $@ -L $(RVO_DIR) -L $(LUA_STATICLIB)
# lua
LUA_STATICLIB := ../../skynet/3rd/lua/liblua.a
LUA_INC ?= ../../skynet/3rd/lua

# rvo
RVO_DIR = $(LUALIB_SRC)
RVO_OBJS = $(RVO_DIR)/Agent.o $(RVO_DIR)/KdTree.o $(RVO_DIR)/Obstacle.o $(RVO_DIR)/RVOSimulator.o $(RVO_DIR)/lua-rvo.o
RVO_TARGET = rvo.so

all: $(RVO_TARGET)

.PHONY: all clean

.c.o:
	$(CC) -c $(SHARED) $(CFLAGS) -o $@ $<

%.o:%.cpp
	$(CC) -c $(SHARED) -o $@ $<

$(RVO_TARGET): $(RVO_OBJS)
ifeq ($(uname_S), Darwin)	
	$(CC) $(SHARED) $(RVO_CFLAGS) $^ -o $@ -L $(LUA_STATICLIB)	
else
	$(CC) $(SHARED) $(CFLAGS) $^ -o $@ -L $(LUA_STATICLIB)
endif

install:
	cp -rf $(RVO_TARGET) $(LUA_CLIB_DIR)

clean:
	rm -f $(RVO_DIR)/*.o 
	rm -f $(RVO_DIR)/*.so 
