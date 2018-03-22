#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <sstream>

#pragma comment (lib, "ws2_32.lib")

using std::cerr;
using std::endl;
using std::cout;
using std::string;
using std::ostringstream;

void main()
{
	// Initialze winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0)
	{
		cerr << "Can't Initialize winsock!" << endl;
		return;
	}

	// Create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		cerr << "Can't create a Socket!" << endl;
		return;
	}

	// Bind the ip address and port to a socket
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY; // Could also use inet_pton .... 

	bind(listening, (sockaddr*)&hint, sizeof(hint));

	// Tell Winsock the socket is for listening 
	listen(listening, SOMAXCONN);

	fd_set master;
	FD_ZERO(&master);

	FD_SET(listening, &master);

	while (true) {

		fd_set copyMaster = master;

		int socketCount = select(0, &copyMaster, nullptr, nullptr, nullptr);

		for (int i = 0; i < socketCount; i++) {
			
			SOCKET sock = copyMaster.fd_array[i];

			if (sock == listening) {
				//Accept a new connection
				SOCKET client = accept(listening, nullptr, nullptr);

				//Add the new connection to the list of connected clients
				FD_SET(client, &master);

				//Send a welcome msg to the client
				string welcomeMsg = "Welcome to the Chat Server!\r\n";
				send(client, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0);

			}
			else {
				
				//Create a buffer
				char buf[4096];
				ZeroMemory(buf, 4096);

				//Receive Message
				int bytesIn = recv(sock, buf, 4096, 0);

				if (bytesIn <= 0) {
					//Drop Client
					closesocket(sock);
					FD_CLR(sock, &master);
				}
				else {
					//Send message to other clients
					for (int i = 0; i < master.fd_count; i++) {
						SOCKET outSock = master.fd_array[i];

						if (outSock != listening && outSock != sock) {

							ostringstream ss;
							ss << "SOCKET #" << sock << ": " << buf << "\r\n";
							string strOut = ss.str();

							send(outSock, strOut.c_str(), strOut.size() + 1, 0);
						}
					}
				}
			}
		}
	}
	
	// Cleanup winsock
	WSACleanup();

	system("pause");
}