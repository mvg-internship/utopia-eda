FLEX_INPUT = Lexer.l
FLEX_HEADER = lexer.h
FLEX_SOURCE = lex.yy.c

SOURCE = Parser_test.cpp
OBJECT = $(SOURCE:.cpp=.o)
TARGET = parser

CXXFLAGS += \
  -std=c++17 -g -Wall -Wextra

all: $(TARGET)

$(TARGET): $(OBJECT)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<


$(FLEX_SOURCE) $(FLEX_HEADER): $(FLEX_INPUT)
	flex --header-file=$(FLEX_HEADER) -o $(FLEX_SOURCE) $<

clean:
	rm -rf $(TARGET) $(OBJECT) $(FLEX_SOURCE) $(FLEX_HEADER)

ifneq (clean, $(MAKECMDGOALS))
-include .deps.mk
endif

.deps.mk: $(SOURCE) Makefile
	$(CXX) -MM -MG $^ > $@




