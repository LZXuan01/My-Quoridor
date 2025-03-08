#ifndef CLIENT_H
#define CLIENT_H

#include <string>

extern char clientName[256];
extern int actionType  ;
extern int x , y ;
extern bool isHorizontal ;
extern int currentTurn ;

const char *getClientName(); // 获取clientName函数（ room.cpp to client.cpp)

extern std::string GameMessage ;



// **启动客户端线程（非阻塞）**
void startClientThread();

std::string getClientID();
std::string getOpponentName();

#endif  
