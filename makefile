HW3_102011252_Ser:HW3_102011252_Ser.cpp HW3_102011252_Cli.cpp
	g++ HW3_102011252_Ser.cpp -o server -lpthread
	g++ HW3_102011252_Cli.cpp -o client -lpthread
clean:
	rm -f server client