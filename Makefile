CC = g++
CFLAGS = -std=c++11 -Wall -Wextra
LDFLAGS = -lX11  -lXtst  `pkg-config --cflags opencv4` `pkg-config --libs opencv4`

TARGET = find_template

all: $(TARGET)

$(TARGET): main.cpp
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

.PHONY: clean

clean:
	rm -f $(TARGET)
