OBJ = main.o eventlist.o parser.o tests.o 
BIN = validator

all: $(OBJ)
	$(CXX) -o $(BIN) $(OBJ)

%.o: %.cpp
	$(CXX) -c -Wall $<

clean:
	rm -rf $(BIN) $(OBJ)
