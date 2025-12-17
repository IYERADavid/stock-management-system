CC = gcc
CFLAGS = -Wall -std=c99
GTK_FLAGS = `pkg-config --cflags --libs gtk+-3.0`
TARGET = StockManager
SRCDIR = src
SOURCES = $(SRCDIR)/main.c $(SRCDIR)/gui.c $(SRCDIR)/product.c $(SRCDIR)/file.c $(SRCDIR)/stock.c $(SRCDIR)/report.c
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(GTK_FLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(GTK_FLAGS)

clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -f data/temp.dat

install-deps:
	@echo "Installing dependencies..."
	@xcode-select --install || true
	@brew install gtk+3 || echo "Please install GTK+3 manually: brew install gtk+3"

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean install-deps run

