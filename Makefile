TARGET_DIR = bin
CXX = g++
CXXFLAGS =  -O3 -std=c++17  -Wall -Wextra  -MMD -MP -fopenmp
SRC_DIR = src
LDFLAGS := -lglfw -lGL -lGLEW

TARGET = $(TARGET_DIR)/volumen
SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
# all ビルドルール
all: $(TARGET)

# TARGET の依存関係ルール
$(TARGET): $(SOURCES) 
	@mkdir -p $(TARGET_DIR)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)


# clean
clean:
	rm -f $(TARGET)

