/*
 * common.h
*
 */

#ifndef PUDSUB_HEADER_COMMON_H_
#define PUDSUB_HEADER_COMMON_H_


#define MAX_EVENTS 50
#define MAX_CLIENT 20

typedef void ConContext;

typedef int (*receivecallback)(char *buf, int fd);

enum ConnectionType {
    NONE,
    PUBLISH,
    SUBSCRIBE,
    PUBLISH_SUBSCRIBE,
    SERVER,
    CLIENT
};

class Addresses
{
public:
    Addresses()
    {
        memset(m_ip, 0 , sizeof(m_ip));
        m_port = 0;
        m_id = -1;
    }
    Addresses(char *ip, uint16_t port, uint16_t id):
        m_port(port), m_id(id)
    {
        memcpy(m_ip, ip , sizeof(m_ip));
    }
    char m_ip[16];
    uint16_t m_port;
    uint16_t m_id;
};

enum SocketState {
    INIT,
    SOCKET_CREATED,
    BIND,
    LISTENING,
    CONNECT_WAIT,
    CONNECTED,
    ERROR
};

#endif /* PUDSUB_HEADER_COMMON_H_ */
