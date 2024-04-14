PROCESS_DIR?=./process
INCLUDES += -I$(PROCESS_DIR)/include

PROCESS_OBJS:= \
	objs/process_buffer.o \
	objs/process_output.o \
	objs/process_pipe.o \
	objs/process_stream.o \
	objs/process.o

objs/process_buffer.o: $(PROCESS_DIR)/src/buffer.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

objs/process_output.o: $(PROCESS_DIR)/src/output.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

objs/process_pipe.o: $(PROCESS_DIR)/src/pipe.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

objs/process_stream.o: $(PROCESS_DIR)/src/stream.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;

objs/process.o: $(PROCESS_DIR)/src/process.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<;
