#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080

struct ClientInfo {
    SOCKET socket;
    std::string username;
};

std::vector<ClientInfo> clients;
CRITICAL_SECTION clients_lock;

// Broadcast message to all clients except sender
void broadcastMessage(const std::string& msg, SOCKET sender) {
    EnterCriticalSection(&clients_lock);
    for (auto& client : clients) {
        if (client.socket != sender) {
            send(client.socket, msg.c_str(), msg.length(), 0);
        }
    }
    LeaveCriticalSection(&clients_lock);
}

// Thread function to handle a single client
DWORD WINAPI handleClient(LPVOID param) {
    SOCKET clientSocket = ((ClientInfo*)param)->socket;
    std::string username = ((ClientInfo*)param)->username;
    char buffer[1024];

    std::string joinMsg = username + " has joined the chat!";
    broadcastMessage(joinMsg, clientSocket);
    std::cout << joinMsg << std::endl;

    while (true) {
        int bytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            EnterCriticalSection(&clients_lock);
            clients.erase(std::remove_if(clients.begin(), clients.end(),
                        [clientSocket](ClientInfo& c){ return c.socket == clientSocket; }),
                        clients.end());
            LeaveCriticalSection(&clients_lock);

            std::string leaveMsg = username + " has left the chat!";
            broadcastMessage(leaveMsg, clientSocket);
            std::cout << leaveMsg << std::endl;

            closesocket(clientSocket);
            break;
        }
        buffer[bytes] = '\0';
        std::string msg = username + ": " + buffer;
        std::cout << msg << std::endl;
        broadcastMessage(msg, clientSocket);
    }

    delete (ClientInfo*)param;
    return 0;
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }

    InitializeCriticalSection(&clients_lock);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        std::cerr << "Listen failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server started on port " << PORT << std::endl;

    while (true) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) continue;

        char nameBuffer[50];
        send(clientSocket, "Enter your username: ", 21, 0);
        int n = recv(clientSocket, nameBuffer, sizeof(nameBuffer)-1, 0);
        if (n <= 0) {
            closesocket(clientSocket);
            continue;
        }
        nameBuffer[n] = '\0';
        std::string username(nameBuffer);

        ClientInfo* ci = new ClientInfo{clientSocket, username};

        EnterCriticalSection(&clients_lock);
        clients.push_back(*ci);
        LeaveCriticalSection(&clients_lock);

        CreateThread(nullptr, 0, handleClient, ci, 0, nullptr);
    }

    closesocket(serverSocket);
    DeleteCriticalSection(&clients_lock);
    WSACleanup();
    return 0;
}
