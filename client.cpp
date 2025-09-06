#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
SOCKET sock;

// Thread function to receive messages
DWORD WINAPI receiveMessages(LPVOID param) {
    char buffer[1024];
    while (true) {
        int bytes = recv(sock, buffer, sizeof(buffer)-1, 0);
        if (bytes <= 0) break;
        buffer[bytes] = '\0';
        std::cout << buffer << std::endl;
    }
    return 0;
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed.\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // Receive prompt for username
    char buffer[50];
    int n = recv(sock, buffer, sizeof(buffer)-1, 0);
    buffer[n] = '\0';
    std::cout << buffer;
    
    std::string username;
    std::getline(std::cin, username);
    send(sock, username.c_str(), username.length(), 0);

    CreateThread(nullptr, 0, receiveMessages, nullptr, 0, nullptr);

    std::string msg;
    while (true) {
        std::getline(std::cin, msg);
        if (msg == "/exit") break;
        send(sock, msg.c_str(), msg.length(), 0);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
