CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = order_book_sim
SOURCES = src/main.cpp src/order_book.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

test: tests/test_order_book.cpp src/order_book.cpp
	$(CXX) $(CXXFLAGS) -o test_runner tests/test_order_book.cpp src/order_book.cpp
	./test_runner

clean:
	rm -f $(TARGET) test_runner
