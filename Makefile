CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LDFLAGS = -lsystemd

TARGET = systemd_control
SRC = systemd_control.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean 