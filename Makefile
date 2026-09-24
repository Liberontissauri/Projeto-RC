COMPILER=g++
EXECUTABLE=user
SOURCES=app.cpp
OBJECTS=$(SOURCES:.cpp=.o)

all: $(SOURCES) $(EXECUTABLE)

.cpp.o:
	$(COMPILER) -std=c++17 -Wall -Wextra -c $< -o $@

$(EXECUTABLE): $(OBJECTS)

	$(COMPILER) $(OBJECTS) -o $(EXECUTABLE)

clean: 
	rm -f *.o $(EXECUTABLE)