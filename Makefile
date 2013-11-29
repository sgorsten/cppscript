all: app

clean:
	rm -f bin/app

app: src/app.cpp src/cppscript.cpp src/cppscript.h
	g++ -std=c++11 src/*.cpp -ldl -o bin/app

test: app
	bin/app
