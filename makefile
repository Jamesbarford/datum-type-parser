OUT_DIR			= output
SRC 			= ./
LINK_TARGET 	= $(OUT_DIR)/main.out
SRC_FILES 		= $(shell find $(SRC) -name '*.cpp')
OBJS 			= $(patsubst $(SRC)/%.cpp, $(OUT_DIR)/%.o, $(SRC_FILES))
REBUILDABLES	= $(OBJS) $(LINK_TARGET)
CC_FLAGS		= -std=c++17 -Wall -g -O0
CC				= clang++

all: $(LINK_TARGET)
	@echo "compilation success ✅"

$(LINK_TARGET): $(OBJS)
	$(CC) $(CC_FLAGS) -o $@ $^

$(OUT_DIR)/%.o: $(SRC)/%.cpp
	$(CC) $(CC_FLAGS) -o $@ -c $<

clean:
	rm -rf $(OUT_DIR)/*
	@echo "clean done ✨"

init:
	mkdir -p output
	@$(MAKE)

run:
	./output/main.out
