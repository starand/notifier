TARGET = notifier

FW = ../../../cpp_fw/
TP = ../../../cpp/3rdparty
JSON = ../../../cpp/3rdparty/json
SRC = .
COMMON = ../common
BIN = ../../bin

SOURCES = \
	$(SRC)/client_handler.cpp \
	$(SRC)/db_handler.cpp \
	$(SRC)/net_server.cpp \
	$(SRC)/notifier_app.cpp \
	$(FW)/common/logger.cpp \
	$(FW)/utils/threading/threading.cpp \
	$(FW)/utils/threading/threading_linux.cpp \
	$(FW)/utils/net/xsocket.cpp \
	$(FW)/utils/files/ini_parser.cpp \
	$(FW)/utils/utils.cpp \
	$(FW)/utils/strutils.cpp \
	$(FW)/utils/files/fileutils.cpp \
	$(FW)/utils/files/config.cpp \
	$(FW)/utils/db/mysql_connection.cpp \
	$(FW)/utils/db/mysql_query.cpp  \
	$(TP)/json/json_reader.cpp \
	$(TP)/json/json_writer.cpp \
	$(TP)/json/json_value.cpp

OBJECTS = $(subst .cpp,.o, $(SOURCES))

INCLUDES = -I. \
        -I.. -I$(COMMON) -I$(FW) \
	-I$(FW)/types -I$(FW)/common -I$(FW)/utils -I$(FW)/utils/threading -I$(FW)/utils/net \
	-I$(FW)/utils/files -I$(FW)/utils/db \
	-I/usr/include/mysql \
	-I$(TP)

CXX=g++
CXXFLAGS = $(INCLUDES) \
	-c -g -std=gnu++0x

LDFLAGS = -lpthread -lrt \
	-L/usr/local/mysql/lib -lmysqlclient -lz

$(TARGET): $(OBJECTS)
	-@mkdir -p $(BIN)
	@echo "  [$(TARGET)]"
	@$(CXX) $(LDFLAGS) $(OBJECTS) -o $(BIN)/$(TARGET)

.cpp.o:
	@echo "    $< .."
	@$(CXX) $(CXXFLAGS) -o $@ $<

clean :
	-@rm -rf $(OBJECTS) $(BIN)/$(TARGET)

rebuild: clean $(TARGET)
	-@echo REBUILD

run: $(TARGET)
	-@$(BIN)/$(TARGET)

