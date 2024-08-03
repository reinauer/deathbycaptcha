CC = gcc
CFLAGS_REQ = -pedantic -Os -mtune=generic
CFLAGS_LIB =
CFLAGS =
LDFLAGS =
LDFLAGS_REQ = -lm
LDFLAGS_LIB =

NAME = deathbycaptcha

OBJ = src/base64.o src/cJSON.o src/deathbycaptcha.o
CLIENT_SRC = src/client.c


WIN32 = 0
ifeq (1,$(WIN32))
	CFLAGS_LIB += -mdll
	LDFLAGS_REQ += -lws2_32
	LDFLAGS_LIB += -Wl,--out-implib,$(LIB:.dll=.lib)
	CLIENT = bin/$(NAME).exe
	LIB = lib/$(NAME).dll
	MYADTOOLS_OBJ = src/MyAdTools/DeathByCaptcha.o
	MYADTOOLS_LIB = lib/MyAdTools/DeathByCaptcha.dll
else
	CFLAGS_REQ += -fPIC -fvisibility=hidden
	CFLAGS_LIB += -shared
	LDFLAGS_REQ += -lpthread
	CLIENT = bin/$(NAME)
	LIB = lib/lib$(NAME).so
endif


all: clean lib client package


clean:
	-find src -type f -name \*.o -delete
	rm -Rf bin/
	rm -Rf lib/

.PHONY: lib
lib:
lib: $(OBJ)
	-mkdir -p lib
	$(CC) $(CFLAGS_REQ) $(CFLAGS) $(CFLAGS_LIB) -o $(LIB) $(OBJ) $(LDFLAGS) $(LDFLAGS_REQ) $(LDFLAGS_LIB)

client:
client: $(OBJ)
	-mkdir -p bin
	$(CC) $(CFLAGS_REQ) $(CFLAGS) -o $(CLIENT) $(OBJ) $(CLIENT_SRC) $(LDFLAGS) $(LDFLAGS_REQ)

myadtools_lib:
myadtools_lib: $(OBJ) $(MYADTOOLS_OBJ)
	-mkdir -p lib/MyAdTools
	$(CC) $(CFLAGS_REQ) $(CFLAGS) $(CFLAGS_LIB) -o $(MYADTOOLS_LIB) $(OBJ) $(MYADTOOLS_OBJ) $(LDFLAGS) $(LDFLAGS_REQ) -Wl,--out-implib,$(MYADTOOLS_LIB:.dll=.lib)

package:
	-rm -f bin/dbc_api_c.zip
	zip -9rX bin/dbc_api_c.zip doc src Makefile -x \*/.svn/\* -x \*.o


$(OBJ) $(MYADTOOLS_OBJ): %.o: %.h %.c
	$(CC) $(CFLAGS_REQ) $(CFLAGS) -c -o $@ $(@:.o=.c)
