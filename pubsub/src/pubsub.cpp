/*
 * pubsub.cpp
 *
 */

#include "../header/pubsub.h"


ConContext* createConnection(const ConnectionType &type, Addresses &serverAddr, std::vector<Addresses> &clientAddrList, uint32_t noOfClient, receivecallback callback)
{
    bool status = true;;
    if(noOfClient > 50)
    {
        std::cout<<"more than 50 clients are not suported.\n";
        return NULL;
    }

    ConnectionHandler *handler = new ConnectionHandler(type, callback);
    handler->m_server.insert(std::pair<uint16_t, Server>(serverAddr.m_id ,Server(serverAddr)));

    std::set<uint16_t> clientids;

    for(uint16_t i = 0; i<noOfClient; ++i )
    {
        //clientAddrList[i].createClient(handler);
        handler->m_clients.insert(std::pair<uint16_t, Connection>(clientAddrList[i].m_id,Connection(clientAddrList[i])));
        clientids.insert(clientAddrList[i].m_id);
    }

    Server &serv = handler->m_server[serverAddr.m_id];

    if(type == ConnectionType::PUBLISH_SUBSCRIBE)
    {
        std::cout<<" PUBLISH_SUBSCRIBE \n";
        serv.createServer(handler);

        for(auto &id : clientids)
        {
            handler->m_clients[id].createClient(handler);
        }
    }
    else if(type == ConnectionType::PUBLISH)
    {

        serv.createServer(handler);

    }
    else if(type == ConnectionType::SUBSCRIBE)
    {

        for(auto &id : clientids)
        {
            handler->m_clients[id].createClient(handler);
        }
    }
    else if(type == ConnectionType::CLIENT)
    {
        for(auto &id : clientids)
        {
            handler->m_clients[id].createClient(handler);
        }
    }
    else if(type == ConnectionType::SERVER)
    {
        serv.createServer(handler);
    }
    else
    {
        std::cout<<"invalid connection type \n";
        status = false;
    }
    if(status)
    {
        handler->startThread();
    }

    return handler;
}


int subscribeMessage(ConContext* handler, std::vector<MsgType>& type,   std::vector<Addresses> &clientAddrList, uint32_t noOfClient)
{
    std::cout<<"message subscribed \n";
    for(uint16_t i = 0; i< noOfClient; ++i)
    {
        ((ConnectionHandler*)handler)->m_clients[clientAddrList[i].m_id].unSubscribed.insert(type.begin(), type.end());
        std::cout<<"subscribed size: "<< ((ConnectionHandler*)handler)->m_clients[clientAddrList[i].m_id].unSubscribed.size();
    }

    return 0;
}

int sendMessage(std::shared_ptr<Message> &msg, ConContext* handler)
{
    ((ConnectionHandler*)handler)->sendMessage(msg);
    return 0;
}
