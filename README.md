TCP Chat App

Multi-client TCP Chat Application built in C++ for Windows using Win32 threads and CRITICAL_SECTION.

Features
1. Multi-client real-time chat using TCP sockets.
2. Unique usernames for each client.
3. Join/leave announcements to notify all participants.
4. Messages are broadcasted to all connected clients.
5. Fully terminal-based and lightweight.

How to Run
1. Compile server.cpp and client.cpp using MinGW:
g++ server.cpp -o server.exe -lws2_32
g++ client.cpp -o client.exe -lws2_32
2. Run the server first:
./server.exe
3. Run one or more clients in separate terminals:
./client.exe
4. Enter your username when prompted and start chatting.
5. Type /exit to leave the chat.

Requirements
1. Windows OS
2. MinGW or Visual Studio with C++ support
