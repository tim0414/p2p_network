HW3_102011252_Ser:HW3_102011252_Ser.cpp HW3_102011252_Cli.cpp connection.cpp client2.cpp upload.cpp
	g++ HW3_102011252_Ser.cpp -o server -lpthread
	g++ HW3_102011252_Cli.cpp connection.cpp -o client -lpthread
	g++ client2.cpp connection.cpp upload.cpp p2p_download.cpp -o client2 -lpthread
	
clean:
	rm -f server client