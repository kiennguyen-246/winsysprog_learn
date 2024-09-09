run:
	g++ -std=c++23 -I./include -DBUILD_LIB -Wall -shared lib1.cpp -o ./lib1.dll
	g++ -std=c++23 -I./include -L. -llib1 -Wall -municode main.cpp -o main.exe
	./main