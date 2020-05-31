/*
 * message.h
*
 */

#pragma once
#ifndef PUDSUB_HEADER_MESSAGE_H_
#define PUDSUB_HEADER_MESSAGE_H_

#define HEADER_LENGTH 6
enum MsgType
{
    SUBSCRIPTION_REQ,
    SUBSCRIPTION_REFRESH,
    SUBSCRIPTION_ACK,
    TEST_MESSAGE,
    TEST1_MESSAGE,
    TEST2_MESSAGE,
    TEST3_MESSAGE
};

struct Message
{
    MsgType type;
    uint32_t length;
    char data[];
};

#endif /* PUDSUB_HEADER_MESSAGE_H_ */
