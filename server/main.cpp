#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <map>
#include <vector>

#define LOCAL_IP "127.0.0.1"
#define PORT 3999
#define MAX_QUEUE 10

using namespace std;
typedef map<SOCKET,string> myMap;
myMap clients;

void broadcastToClients(string msg, vector<SOCKET> except){
  map<SOCKET,string>::iterator j = clients.begin();
  bool skip = false;
  while(j != clients.end()){
      if(!except.empty()){
        for(SOCKET ex : except){
          if(ex == j->first){
            skip = true;
            break;
          }
        }
    }
    if(skip == true) continue;
    int sendRet = send(j->first,msg.c_str(),msg.length(),0);
    j++;
  }
  return;
}

DWORD WINAPI ThreadForClient(LPVOID param){
  SOCKET clientSock = (SOCKET) param;
  char name[4096];
  string question = "[Server] What is your name?";
  int sendRet = send(clientSock,question.c_str(),question.length(),0);
  int recvRet = recv(clientSock,name,4096,0);
  cout << "[info] Client with the name '" << string(name) << "' is typing." << endl;
  vector<SOCKET> exceptSockets;
  exceptSockets.push_back(clientSock);
  broadcastToClients("[Server] Client '" + string(name) + "' joined the chat.", exceptSockets);
  exceptSockets.clear();
  clients.insert(pair<SOCKET,string>(clientSock,name));
  string welcome = "[Server] Welcome to the server, " + string(name) + "! Proceed to chat.";
  sendRet = send(clientSock,welcome.c_str(),welcome.length(),0);
  while(true){
  char buff[4096];
  int recvRet = recv(clientSock,buff,4096,0);
  if(recvRet < 0){
      broadcastToClients("[Server] Client '" + string(name) + "' has left the chat.", vector<SOCKET>());
      cout << "[info] Client with the name '" << string(name) << "' is no longer typing." << endl;
      clients.erase(clientSock);
      fill(buff, buff+4096,'\0');
      break;
  }
  myMap::iterator i = clients.begin();
  while(i != clients.end()){
    string messageOut = string(name) + ": " + buff;
    if(i->first != clientSock){
      sendRet = send(i->first,messageOut.c_str(),messageOut.length(),0);
    }else{
      cout << messageOut << endl;
    }
    i++;
  }
  fill(buff, buff+4096,'\0');
  }
  return 0;
}

int main() {
  WSADATA wsData;
  DWORD wsVersion = MAKEWORD(2,2);
  if(WSAStartup(wsVersion, &wsData) != 0){
    cout << "[error] Failed to initialize Windows Socket." << endl;
    WSACleanup();
    return -1;
  }
  cout << "[info] Started Winsocket successfully." << endl;
  
  SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  cout << "[info] Created socket successfully." << endl;

  if(sock == INVALID_SOCKET){
    cout << "[error] Failed to initialize socket!" << endl;
    closesocket(sock);
    WSACleanup();
    return -1;
  }
  sockaddr_in sockaddr;
  sockaddr.sin_family = AF_INET;
  inet_pton(AF_INET, LOCAL_IP, &sockaddr.sin_addr);
  sockaddr.sin_port = htons(PORT);

  int bindRet = bind(sock, (struct sockaddr*) &sockaddr, sizeof(sockaddr));
  if(bindRet < 0){
    cout << "[error] Binding failed!" << endl;
    closesocket(sock);
    WSACleanup();
    return -1;
  }
  cout << "[info] Socket was binded successfully. Ret: " << bindRet << endl;
  int listRet = listen(sock,MAX_QUEUE);
  cout << "[info] Waiting for connection. Ret: " << listRet << endl;
  while(true){
  SOCKET clientSock = accept(sock,NULL,NULL);
  if(clientSock == INVALID_SOCKET){
    continue;
  }
  cout << "[info] Client connected!" << endl;
  HANDLE thread;
  DWORD dwThreadID;
  CreateThread(NULL, 0, ThreadForClient, (LPVOID)clientSock,0, &dwThreadID);
  }
  cout << "[info] Cleaning up. Ret: " << WSAGetLastError() << endl;
  clients.clear();
  closesocket(sock);
  WSACleanup();
  
  return 0;
}