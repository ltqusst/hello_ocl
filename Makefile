SRC = main.cpp utils.cpp run_hello.cpp run_myGEMM.cpp

all: hello

hello: $(SRC) 
	g++ -o $@ $^ -lOpenCL


