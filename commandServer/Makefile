run:
	g++ -std=c++23 -municode -g -Wall NPServer.cpp -o NPServer.exe
	g++ -std=c++23 -municode -g -Wall NPClient.cpp -o NPClient.exe
	./NPServer.exe
	./NPClient.exe

run_server:
	g++ -std=c++23 -municode -g -Wall NPServer.cpp -o NPServer.exe
	./NPServer.exe

run_client:
	g++ -std=c++23 -municode -g -Wall NPClient.cpp -o NPClient.exe
	./NPClient.exe

build_utils:
	g++ -std=c++23 -municode -g -Wall add.cpp -o add.exe
	g++ -std=c++23 -municode -g -Wall sub.cpp -o sub.exe
	g++ -std=c++23 -municode -g -Wall mul.cpp -o mul.exe
	g++ -std=c++23 -municode -g -Wall div.cpp -o div.exe

clean:
	del /q Temp
	# del NPClient.exe
	# del NPServer.exe