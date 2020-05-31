/*
 * pubsub.cpp
 *
 */
#pragma once
#ifndef PUDSUB_HEADER_PUBSUB_H_
#define PUDSUB_HEADER_PUBSUB_H_


#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <memory>
#include "../header/common.h"
#include "../header/connection.h"
#include "../header/message.h"
#include<set>


extern "C"
{

ConContext* createConnection(const ConnectionType &type, Addresses &serverAddr, std::vector<Addresses> &clientAddrList, uint32_t noOfClient, receivecallback callback);
int sendMessage(std::shared_ptr<Message> &msg, ConContext* handler);
int receiveMessage(Message *msg, ConContext* handler);
int subscribeMessage(ConContext* handler, std::vector<MsgType>& type,  std::vector<Addresses> &clientAddrList, uint32_t noOfClient);
}
#endif
