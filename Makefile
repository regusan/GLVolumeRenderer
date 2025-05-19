TARGET_DIR = bin
OBJ_DIR = obj
CXX = g++
CXXFLAGS = -O0 -g -std=c++17 -Wall -Wextra -MMD -MP -fopenmp
SRC_DIR = src
LDFLAGS := -lglfw -lGL -lGLEW

TARGET = $(TARGET_DIR)/volumen
SOURCES := $(shell find $(SRC_DIR) -name '*.cpp')
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SOURCES))
DEPENDS := $(OBJECTS:.o=.d)

# all ビルドルール
all: $(TARGET)

# TARGET の依存関係ルール
$(TARGET): $(OBJECTS)
	@mkdir -p $(TARGET_DIR)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# オブジェクトファイル生成ルール
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 依存関係ファイルを含める
-include $(DEPENDS)

# clean
clean:
	rm -rf $(TARGET) $(OBJ_DIR)
