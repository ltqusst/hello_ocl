SRC = main.cpp utils.cpp run_hello.cpp run_myGEMM.cpp run_myNV12toBGR24.cpp

all: hello

hello: $(SRC) 
	g++ -o $@ $^ -lOpenCL


