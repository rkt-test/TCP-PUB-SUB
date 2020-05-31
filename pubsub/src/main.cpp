/*
 * main.cpp
 *
 */
#include "../header/pubsub.h"
#include "../header/common.h"

int receiveMessage(char *buf, int fd)
{
    Message* msg = (Message*)buf;
    std::cout<<" fd : "<<fd <<" msg Type: "<<msg->type<<"\n";
}

int main()
{
    char ip[16] = "127.0.0.1";
    Addresses serv(ip, 5000, 1);
    std::vector<Addresses> clients;
    clients.push_back(Addresses(ip, 5001, 2));
    clients.push_back(Addresses(ip, 5002, 3));
    clients.push_back(Addresses(ip, 5003, 4));

    ConContext* context1 = createConnection(ConnectionType::PUBLISH_SUBSCRIBE, serv, clients,
                                           clients.size(), &receiveMessage);

    Addresses serv1(ip, 5001, 7);
    std::vector<Addresses> clients1;
    clients1.push_back(Addresses(ip, 5000, 8));
    clients1.push_back(Addresses(ip, 5002, 5));
    clients1.push_back(Addresses(ip, 5003, 6));

    ConContext* context2 = createConnection(ConnectionType::PUBLISH_SUBSCRIBE, serv1, clients1,
                                           clients1.size(), &receiveMessage);

    Addresses serv2(ip, 5002, 15);
    std::vector<Addresses> clients2;
    clients2.push_back(Addresses(ip, 5000, 9));
    clients2.push_back(Addresses(ip, 5001, 10));
    clients2.push_back(Addresses(ip, 5003, 11));

    ConContext* context3 = createConnection(ConnectionType::PUBLISH_SUBSCRIBE, serv2, clients2,
                                           clients2.size(), &receiveMessage);

    Addresses serv3(ip, 5003, 16);
    std::vector<Addresses> clients3;
    clients3.push_back(Addresses(ip, 5000, 12));
    clients3.push_back(Addresses(ip, 5001, 13));
    clients3.push_back(Addresses(ip, 5002, 14));

    ConContext* context4 = createConnection(ConnectionType::PUBLISH_SUBSCRIBE, serv3, clients3,
                                           clients3.size(), &receiveMessage);

    std::vector<MsgType> type;
    type.push_back(TEST1_MESSAGE);
    type.push_back(TEST2_MESSAGE);
    type.push_back(TEST3_MESSAGE);

    if(context1 != NULL && context2 != NULL && context3 != NULL && context4 != NULL)
    {
        int s = subscribeMessage(context1, type, clients, clients.size());
        int s1 = subscribeMessage(context2, type, clients1, clients1.size());
        int s2 = subscribeMessage(context3, type, clients2, clients2.size());
        int s3 = subscribeMessage(context4, type, clients3, clients3.size());
    }
    else
    {
        std::cout<<"\n connnection creation failed.\n";
    }

    while(1)
    {
    }

}
