#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#define LOCAL_IP "127.0.0.1"
#define PORT 3999

using namespace std;
bool canSendMsg = false;
bool namePrompt = false;
string name = "";


DWORD WINAPI ReadingThread(LPVOID param){
    SOCKET sock = (SOCKET) param;
    char buff[4096];
    while(true){
    int recvRet = recv(sock,buff,4096,0);
    if(recvRet < 0){
        continue;
    }
    if(string(buff).compare("[Server] What is your name?") == 0){
        canSendMsg = true;
    }
    cout << buff << endl;
    fill(buff, buff+4096,'\0');
    }
    return 0;
}

int main(){
    canSendMsg = false;
    WSADATA wsData;
    if(WSAStartup(MAKEWORD(2,2),&wsData) != 0){
        cout << "[error] Failed to start winsock!" << endl;
        WSACleanup();
        return -1;
    }
    cout << "[info] Winsock started." << endl;

    sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    inet_pton(AF_INET, LOCAL_IP, &sockaddr.sin_addr);
    sockaddr.sin_port = htons(PORT);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock == INVALID_SOCKET){
        cout << "[error] Failed to create socket!" << endl;
        closesocket(sock);
        WSACleanup();
        return -1;
    }
    cout << "[info] Socket successfully created." << endl;
    if(connect(sock,(struct sockaddr*)&sockaddr,sizeof(sockaddr)) != 0){
        cout << "[error] Failed to establish connection!" << endl;
        closesocket(sock);
        WSACleanup();
        return -1;
    }
    cout << "[info] Connection established!" << endl;

    HANDLE hThread;
    DWORD dwThreadID;
    hThread = CreateThread(NULL, 0, &ReadingThread, (LPVOID)sock,0,&dwThreadID);
    while(canSendMsg == false){
        
    }
    string inp;
    while(inp != "quit"){
        getline(cin,inp);
        int sendRet = send(sock, inp.c_str(), inp.length(), 0);
        if(sendRet < 0){
            cout << "[error] Lost connection to the server!" << endl;
            break;
        }
    }
    cout << "[info] Cleaning up!" << endl;
    closesocket(sock);
    WSACleanup();
    return 0;
}