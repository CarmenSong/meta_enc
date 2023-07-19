TARGET = dmdkctl
OBJS = dm.o \
       dm_table.o \
       dm_target.o \
       utility.o \
       main.o

CXX = $(CROSS_COMPILE)g++
CXXFLAGS += -std=c++17
.SUFFIXES: .cpp.o

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS)
