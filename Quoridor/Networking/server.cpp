#include "thirdparty/httplib.h"
#include "unordered_map"

#include <mutex>
using namespace std;

unordered_map<string, int> userbook;

std::vector<std::string> message_history;
mutex message_mutex;

int client_id;
int current_client_id = 1;
int connected_clients = 0;

void login(const httplib::Request &req, httplib::Response &res);

void message(const httplib::Request &req, httplib::Response &res);

void get_turn(const httplib::Request &req, httplib::Response &res);

void ready(const httplib::Request &req, httplib::Response &res);

void get_messages(const httplib::Request &req, httplib::Response &res);

int main()
{
    httplib::Server server;
    cout << "Server listening the port 25565..." << endl
         << endl;

    server.Post("/login", login);
    server.Get("/ready", ready);
    server.Post("/message", message);
    server.Get("/turn", get_turn);
    server.Get("/messages",get_messages);

    server.listen("0.0.0.0", 25565); // 所有设备都可以连接此电脑
    return 0;
}



//--------------------------------函数体---------------------------------------------------------------------

//--------------------------------函数体---------------------------------------------------------------------

void login(const httplib::Request &req, httplib::Response &res)
{
    string username = req.body; // 1.从client获取名字

    if (userbook.size() >= 2) // 判断多少个用户
    {
        res.set_content("Server is full.Please try again later", "text/plain");
        return;
    }
    else
    {
        if (userbook.find(username) == userbook.end()) // 如果用户名不存在
        {
            client_id = userbook.size() + 1; // 新的ID
            connected_clients += 1;
            userbook[username] = client_id; // 绑定用户名和 ID
        }
        else
        {
            client_id = userbook[username]; // 获取已存在的 ID
        }

        res.set_content(to_string(client_id), "text/plain"); // 返回ID

        cout << "Client " << client_id << " [" << username << "] connected from " << req.remote_addr << endl;
    }
}

void ready(const httplib::Request &req, httplib::Response &res) // 向用户输出需要等待还是开始
{
    if (connected_clients < 2)
    {
        res.set_content("Waiting", "text/plain"); // client基于waiting这个字来等待opponent名字准没准备好
    }
    else
    {
        string opponentName;
        for (const auto &pair : userbook)
        {
            if (pair.second != stoi(req.get_header_value("Client-ID")))
            {
                opponentName = pair.first;
                break;
            }
        }
        res.set_content(opponentName, "text/plain");
    }
}

void message(const httplib::Request &req, httplib::Response &res)
{
    int client_id = stoi(req.get_header_value("Client-ID")); // 根据client发来的ID知道是哪个client

    if (client_id != current_client_id ) // 检查是否是当前回合的客户端
    {
        res.set_content("Not your turn to send message.", "text/plain");
        return;
    }

    int actionType = stoi(req.get_header_value("Action-Type"));
    int x = stoi(req.get_header_value("X"));
    int y = stoi(req.get_header_value("Y"));
    bool isHorizontal = stoi(req.get_header_value("Is-Horizontal"));

    string message = "Client " + to_string(client_id) + " sent message: " + req.body;
    {
        lock_guard<mutex> lock(message_mutex); // 加锁保护消息历史记录
        message_history.push_back(message);    // 将消息添加到历史记录中
    }

    cout << "Client " << to_string(client_id) << " sent message: " << req.body << endl;

    current_client_id = (current_client_id == 1) ? 2 : 1;
    res.set_content(to_string(current_client_id), "text/plain"); // 返回更新后的回合
}

void get_turn(const httplib::Request &req, httplib::Response &res) // 发送当前回合
{
    res.set_content(to_string(current_client_id), "text/plain");
}

void get_messages(const httplib::Request &req, httplib::Response &res)
{
    lock_guard<mutex> lock(message_mutex); // 加锁保护消息历史记录

    if (!message_history.empty())
    {
        // 只返回最新的消息
        res.set_content(message_history.back(), "text/plain");
    }
    else
    {
        res.set_content("No messages yet.", "text/plain"); // 处理空消息历史记录
    }
}
