/*
  ==============================================================================

	DH.cpp
	Created: 10 Jun 2023 1:12:54pm
	Author:  L

  ==============================================================================
*/

#include "DH.h"

void server(bool flag, void* _t) {
	int ret;
	auto t = reinterpret_cast<DH::LeftGroupComponent*>(_t);
	WSADATA wsa_data;
	SOCKET server_socket, client_socket;
	struct sockaddr_in server_address, client_address;
	int server_port = 13370;

	ret = WSAStartup(MAKEWORD(2, 2), &wsa_data);

	server_socket = socket(AF_INET, SOCK_STREAM, 0);

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(server_port);

	bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));

	listen(server_socket, SOMAXCONN);

	int client_address_size = sizeof(client_address);
	client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_size);

	do {
		ret = recv(client_socket, t->buffer, 20, 0);
		if (ret > 0) {
			auto peerkey = std::stoul(t->buffer);
			auto pubkey = t->server_dh.generatePubkey();
			auto sharedkey = t->server_dh.generateSharedKey(peerkey);
			juce::String tt(pubkey);
			send(client_socket, tt.getCharPointer(), 20, 0);
			t->keyLabel3.setText(juce::String(sharedkey), juce::NotificationType::dontSendNotification);
		}
	} while (flag);
}


void clientSockSetup(void* _t) {
	auto t = reinterpret_cast<DH::RightGroupComponent*>(_t);
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET& ConnectSocket = t->clientSock;
	struct sockaddr_in clientService;

	ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr("127.0.0.1");
	clientService.sin_port = htons(13370);

	connect(ConnectSocket, (SOCKADDR*)&clientService, sizeof(clientService));
}

