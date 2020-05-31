/*
 * connection.h
 *
 */
#pragma once
#ifndef PUDSUB_HEADER_CONNECTION_H_
#define PUDSUB_HEADER_CONNECTION_H_
// #include<pubsub.h>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdint.h>
#include<map>
#include<set>
#include<mutex>
#include<queue>
#include <memory>
#include "../header/common.h"
#include "../header/message.h"
#include <fcntl.h>
#include<thread>



class Connection;
class Server;

class ConnectionHandler
{
public:
    ConnectionHandler(const ConnectionType &type, receivecallback callback);

    ~ConnectionHandler();
    void updateServerFdAndRegFd(int fd, bool add = true);
    void updateClientFdAndRegFd(int fd, uint16_t id, bool add = true);
    void handleEvent(struct epoll_event *event);
    void pollForEventsOnFds();
    int deleteFdFromEpollList(int fd);
    int createAndAddFdForEpoll(uint32_t event, int fd);
    void registerClient(Message* msg, int fd);
    int sendMessage(std::shared_ptr<Message> &msg);
    void sentData(int fd);
    void startThread();



    struct epoll_event ev, events[MAX_EVENTS];
    int epollfd;

    ConnectionType connType;
    char recvBuff[1024];
    char senBuff[1024];
    std::set<int> serverFds;
    std::set<int> clientFds;
    std::mutex mtx;
    receivecallback recCallback;
    std::map<uint16_t, Connection> m_clients;
    std::map<uint16_t, Server> m_server;
    std::map<uint16_t, uint16_t> fdToIdmap;
    std::map<MsgType, std::set<int>> publishedMessage;
    std::map<MsgType, std::set<int>> subscribedMessage;
    std::queue<std::shared_ptr<Message>> sentQueue;
};

class Connection
{
public:
    Connection(char *ip, uint16_t port, uint16_t id):
        m_port(port), m_fd(-1), m_id(id)
    {
        memcpy(m_ipAddr, ip , sizeof(m_ipAddr));
        m_state = SocketState::INIT;
        subscribed.clear();
        unSubscribed.clear();
    }

    Connection(Addresses &addr):
        m_port(addr.m_port), m_fd(-1), m_id(addr.m_id)
    {
        memcpy(m_ipAddr, addr.m_ip , sizeof(m_ipAddr));
        m_state = SocketState::INIT;
        subscribed.clear();
        unSubscribed.clear();
    }

    Connection(){}

    uint16_t formSubscriptionMessage(char *buf)
    {
        //std::cout<<"formSubscriptionMessage ";
        Message* msg = (Message*)buf;
        msg->type = SUBSCRIPTION_REQ;
        msg->length = 0;
        char *data = &(msg->data[0]);


        for(auto type : unSubscribed)
        {
            *data = type;
            //std::cout<< " d: "<<(MsgType)*data<<" type "<<type;
            data += sizeof(MsgType);
            msg->length += sizeof(MsgType);

        }

        return msg->length;
    }

    int createClient(ConnectionHandler *handler);

    uint16_t m_port;
    uint16_t m_id;
    int m_fd;
    SocketState m_state;
    char m_ipAddr[16];
    std::set<MsgType> subscribed;
    std::set<MsgType> unSubscribed;

};

class Server
{
public:
    Server(char *ip, uint16_t port, uint16_t id):
        m_serverConn(ip, port, id)
    {
        m_clients.clear();
    }

    Server(Addresses &addr):m_serverConn(addr)
    {
        m_clients.clear();
    }

    Server(){}
    Connection m_serverConn;

    //<client fd, connection>
    std::map<int, Connection> m_clients;
    int createServer(ConnectionHandler *handler);
};


#endif /* PUDSUB_HEADER_MESSAGE_H_ */

 
 
 
