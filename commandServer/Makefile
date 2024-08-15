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

clean:
	del /q Temp
	# del NPClient.exe
	# del NPServer.exe