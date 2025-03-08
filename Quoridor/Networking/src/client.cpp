#include "../thirdparty/httplib.h"
#include "client.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

using namespace std;


httplib::Client client("192.168.1.107:25565");
std::string last_message = "";
mutex messageMutex;

string username;
string opponent ="" ;
string GameMessage = "";
int client_id = -1;
int currentTurn = -1;



bool check_connection(const httplib::Result &result); //  检查初始状态

string getclientID();

string getOpponentName();

string waitForUsername(); //  停下线程等待获取用户名字

string waitForUserAction();

void fetchMessageThread(); // 获取消息线程

void initClient();

void startClientThread() // 接入点API
{
    thread(initClient).detach();
}


// ------------------------------函数体------------------------------------------------

bool check_connection(const httplib::Result &result) //  检查初始状态
{
    if (!result || result->status != 200)
    {
        cout << "Error: Server connection failed!" << endl;
        return false;
    }
    else
    {
        this_thread::sleep_for(chrono::seconds(1));
        cout << "Success: Server connection successful!" << endl;
        return true;
    }
}

string getClientID()
{
    lock_guard<mutex> lock(messageMutex); // 使用 lock_guard 确保线程安全
    if (client_id == -1)
    {
        return "";
    }
    return to_string(client_id);
}

string getOpponentName()
{
    lock_guard<mutex> lock(messageMutex); // 使用 lock_guard 确保线程安全
    return opponent;
}

string waitForUsername() // 停止线程等待用户名字
{
    std::cout <<  "Waiting for username...\n\n";
    while (strlen(getClientName()) == 0) // 只要用户名还是空的，就等待
    { 
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 每秒检查一次
    }
    std::cout << "Username received: " << getClientName() << std::endl;
    return getClientName();
}

string waitForUserAction()
{

    while(actionType != 1 && actionType != 2 )
    {
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 每秒检查一次
    }
    std::cout << "Your action have been changed succesfully! | ActionType: " << actionType << " | {" << x << " , " << y << "}" << " | isHorizontal: " << isHorizontal << " | " << endl;

    return "ActionType: " + std::to_string(actionType) + " | {" + to_string(x) + " , " + to_string(y) + "}" + " | isHorizontal: " + to_string(isHorizontal) + " | ";
}

void fetchMessageThread()
{
    string last_message = "";
    while (true)
    {
        httplib::Result turn_result = client.Get("/turn");

        if (!turn_result || turn_result->status != 200)
        {
            cout << "cannot connect to server..." << endl;
            this_thread::sleep_for(chrono::milliseconds(1500));
            continue;
        }

        int current_clientID = stoi(turn_result->body);
        currentTurn = current_clientID - 1 ; // get 1 first

        if (client_id == current_clientID)
        {

            httplib::Result messages_result = client.Get("/messages");

            if (messages_result && messages_result->status == 200)
            {
                string new_message = messages_result->body;

                // 如果消息是新的，则显示
                if (new_message != last_message)
                {
                    GameMessage = new_message ;
                    cout <<  new_message << endl;
                    last_message = new_message; // 更新上一次显示的消息

                }
            }

            // Messages to Send

            string message_to_send = waitForUserAction();

            httplib::Headers headers =
            {
                {"Client-ID", to_string(client_id)}, {"Action-Type", to_string(actionType)}, {"X", to_string(x)}, {"Y", to_string(y)}, {"Is-Horizontal", to_string(isHorizontal)}
            };
            httplib::Result res = client.Post("/message", headers, message_to_send , "text/plain"); 
            actionType = 0 ;
            x = 0 ;
            y = 0 ;
        }

        this_thread::sleep_for(chrono::milliseconds(1500));
    }
}

void initClient()
{
    username = waitForUsername();

    // login

    httplib::Result result = client.Post("/login", username, "text/plain");

    if (check_connection(result) == false)
    {
        return;
    }
    client_id = stoi(result->body);
    cout << "Your client ID is " << client_id << endl;

    // Turn & Ready

    bool waiting_printed = false;
    while (true)
    {
        httplib::Headers headers = {{"Client-ID", to_string(client_id)}};
        httplib::Result ready_result = client.Get("/ready", headers);
        if (ready_result && ready_result->body != "Waiting")
        {
            opponent = ready_result->body;
            cout << "Client connection successful, you can start talking." << endl;
            cout << "Your oppnent: " + opponent << endl << endl << endl;
            break;
        }
        if (!waiting_printed)
        {
            cout << "Waiting for other clients to join..." << endl;
            waiting_printed = true;
        }
        this_thread::sleep_for(chrono::seconds(1));
    }

    // Messages

    thread(fetchMessageThread).detach(); // 开始获取信息线程
}
