CC = g++ -std=c++11
#C_ARGS = -O2 -Wall  -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
C_ARGS = -g -O0 -Wall -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE 

THIRD_LIB_BASE = /home/oange/plug/wx/service/wxplug_push/thirdparty

INCLUDE = -I$(THIRD_LIB_BASE)/curl/include -I./ -I$(THIRD_LIB_BASE)/boss/include -I$(THIRD_LIB_BASE)/protobuf/protobuf/include -I$(THIRD_LIB_BASE)/hiredis/include -I$(THIRD_LIB_BASE)/wxstorage/

#SYS_LIBS = -lrt -lz -lm -lnameapi $(THIRD_LIB_BASE)/boss/lib/libbossapi64.a -lpthread -ldl $(THIRD_LIB_BASE)/curl/lib/libcurl.a $(THIRD_LIB_BASE)/protobuf/protobuf/lib/libprotobuf.a $(THIRD_LIB_BASE)/protobuf/protobuf/lib/libprotoc.a $(THIRD_LIB_BASE)/hiredis/lib/libhiredis.a $(THIRD_LIB_BASE)/wxstorage/libwxstorage.a
SYS_LIBS = -lrt -lz -lm $(THIRD_LIB_BASE)/boss/lib/libbossapi64.a -lpthread -ldl $(THIRD_LIB_BASE)/curl/lib/libcurl.a $(THIRD_LIB_BASE)/protobuf/protobuf/lib/libprotobuf.a $(THIRD_LIB_BASE)/protobuf/protobuf/lib/libprotoc.a $(THIRD_LIB_BASE)/hiredis/lib/libhiredis.a $(THIRD_LIB_BASE)/wxstorage/libwxstorage.a -ljsoncpp

BIN = wxplug_helper

.cpp.o:
	$(CC) $(INCLUDE) $(C_ARGS) -c $^ 

.cc.o:
	$(CC) $(INCLUDE) $(C_ARGS) -c $^ 
    
.c.o:
	$(CC) $(INCLUDE) $(C_ARGS) -c $^ 

all: $(BIN)

wxplug_helper : WxPlugHelper.o IniParser.o Log.o 
	$(CC) $^ -o $@ $(INCLUDE) $(C_ARGS) ${SYS_LIBS}
	
clean:
	rm -f *.o $(BIN)

install:
	cp ./$(BIN) ../bin
