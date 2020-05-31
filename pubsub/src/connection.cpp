/*
 * connection.cpp
 *
 */
#include "../header/connection.h"


   ConnectionHandler::ConnectionHandler(const ConnectionType &type, receivecallback callback)
    {
        epollfd = -1;
        serverFds.clear();
        clientFds.clear();
        recCallback = callback;
        connType = type;
        fdToIdmap.clear();
    }

    ConnectionHandler::~ConnectionHandler()
    {
        epollfd = -1;
        serverFds.clear();
        clientFds.clear();
        connType = ConnectionType::NONE;
        fdToIdmap.clear();
    }

int Connection::createClient(ConnectionHandler *handler)
{
    struct sockaddr_in serv_addr; 


    if((m_fd = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0)) < 0)
    {
        std::cout<<"\n Error : Could not create socket \n";
        m_state = SocketState::ERROR;
        return -1;
    } 
    m_state = SocketState::SOCKET_CREATED;

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(m_port);

    if(inet_pton(AF_INET, m_ipAddr, &serv_addr.sin_addr)<=0)
    {
        std::cout<<"\n inet_pton error occured\n";
        m_state = SocketState::ERROR;
        close(m_fd);
        return -1;
    } 

    std::cout<<"\n client going to connect \n";
    if( connect(m_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        if (errno == EINPROGRESS)
        {
            std::cout<<"    connect EINPROGRESS OK ";
        }
        else
        {
            std::cout<<"\n Error : Connect Failed \n";
            m_state = SocketState::ERROR;
            close(m_fd);
            return -1;
        }
    } 

    std::cout<<"\n client created succesfully \n";
    m_state = SocketState::CONNECT_WAIT;
    handler->updateClientFdAndRegFd(m_fd, m_id, true);

    return 0;
}


int Server::createServer(ConnectionHandler *handler)
{
    int connfd = 0;
    int res = 0;
    struct sockaddr_in serv_addr;

    char sendBuff[1025];
    time_t ticks;

    if((m_serverConn.m_fd = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0)) < 0)
    {
        std::cout<<"\n Error : Could not create socket \n";
        m_serverConn.m_state = SocketState::ERROR;
        return 1;
    }

    m_serverConn.m_state = SocketState::SOCKET_CREATED;
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(m_serverConn.m_port);

    if((res = bind(m_serverConn.m_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0)
    {
        std::cout<<"\n Error : bind failed. \n";
        m_serverConn.m_state = SocketState::ERROR;
        close(m_serverConn.m_fd);
        return 1;
    }

    m_serverConn.m_state = SocketState::BIND;

    if((res = listen(m_serverConn.m_fd, MAX_CLIENT)) < 0)
    {
        std::cout<<"\n Error : listen failed. \n";
        m_serverConn.m_state = SocketState::ERROR;
        close(m_serverConn.m_fd);
        return 1;
    }
    std::cout<<"\n server created succesfully \n";
    m_serverConn.m_state = SocketState::LISTENING;
    handler->updateServerFdAndRegFd(m_serverConn.m_fd, true);
}



int ConnectionHandler::createAndAddFdForEpoll(uint32_t event, int fd)
{
    struct epoll_event ev;
    if(epollfd == -1)
    {
        epollfd = epoll_create1(0);
        if (epollfd == -1) {
            std::cout<<"epoll_create1 failed";
            return -1;
        }
    }

    ev.events = event; //EPOLLIN|EPOLLOUT;
    ev.data.fd = fd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        std::cout<<"epoll_ctl failed : added fd: "<< fd;
        return -1;
    }

    std::cout<<"epoll_ctl added fd: "<< fd;

    return 0;
}

int ConnectionHandler::deleteFdFromEpollList(int fd)
{

    if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL) == -1)
    {
        std::cout<<"epoll_ctl: delted fd: "<< fd;
        return -1;
    }
    return 0;
}


void ConnectionHandler::pollForEventsOnFds()
{
    while(1) {
        sleep(2);
        for(auto &cl : m_clients)
        {
            if(cl.second.m_state == SocketState::ERROR)
            {
                std::cout<<"re create clients"<<std::endl;
                cl.second.createClient(this);
            }
            else if(cl.second.m_state == SocketState::CONNECT_WAIT)
            {
                int optval = -1;
                socklen_t optlen = sizeof (optval);

                //check whether connect is success or not.
                if (getsockopt(cl.second.m_fd,
                               SOL_SOCKET, SO_ERROR, &optval,
                               &optlen) == -1)
                {
                    cl.second.m_state== SocketState::ERROR;
                    std::cout<<"\ngetsockopt errorconnected failed on ip: "<<cl.second.m_ipAddr
                            <<"port: "<< cl.second.m_port<< " socket: "<< cl.second.m_fd <<"\n";
                }

                if (optval == 0) {
                    cl.second.m_state == SocketState::CONNECTED;
                    std::cout<<"\nconnected OK on ip: "<<cl.second.m_ipAddr
                            <<"port: "<< cl.second.m_port<< " socket: "<< cl.second.m_fd <<"\n";
                } else {
                    cl.second.m_state== SocketState::ERROR;
                    std::cout<<"\nconnected error on ip: "<<cl.second.m_ipAddr
                            <<"port: "<< cl.second.m_port<< " socket: "<< cl.second.m_fd <<"\n";
                    if(deleteFdFromEpollList(cl.second.m_fd) < 0)
                    {
                        std::cout<<"socket Delete from epoll failed.\n";
                    }
                    close(cl.second.m_fd);
                }
            }
        }
        std::unique_lock<std::mutex> lck (mtx);
        int nfds = epoll_wait(epollfd, events, MAX_EVENTS, 100);
        if (nfds == -1) {
            std::cout<<"epoll_wait error";
            continue;
        }
        lck.unlock();


        for (uint16_t n = 0; n < nfds; ++n) {
            if (serverFds.find(events[n].data.fd) != serverFds.end()) {
                struct sockaddr_in clientAddr;
                socklen_t clientAddr_len = sizeof(struct sockaddr_in);
                int connfd = accept(events[n].data.fd,
                                    (struct sockaddr *) &clientAddr, &clientAddr_len);
                if (connfd == -1) {
                    std::cout<<"accept failed";
                    continue;
                }

                std::cout << "Connection from IP: "
                        << ( ( ntohl(clientAddr.sin_addr.s_addr) >> 24) & 0xff ) << "."  // High byte of address
                        << ( ( ntohl(clientAddr.sin_addr.s_addr) >> 16) & 0xff ) << "."
                        << ( ( ntohl(clientAddr.sin_addr.s_addr) >> 8) & 0xff )  << "."
                        <<   ( ntohl(clientAddr.sin_addr.s_addr) & 0xff ) << ", port "   // Low byte of addr
                        << ntohs(clientAddr.sin_port);

                fcntl(connfd, F_SETFL, O_NONBLOCK);
                ev.events = EPOLLIN|EPOLLOUT;
                ev.data.fd = connfd;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd,
                              &ev) == -1) {
                    std::cout<<"epoll_ctl: add failed connfd: "<<connfd;
                }
            } else {
                handleEvent(&events[n]);
            }
        }
    }
}


void ConnectionHandler::handleEvent(struct epoll_event *event)
{
    int n =0;
    memset(recvBuff, '0',sizeof(recvBuff));
    static bool isread= true;

    if(event->events == EPOLLERR)
    {
        std::cout<<"handleEvent error : \n";
        auto it = fdToIdmap.find(event->data.fd);
        if(it != fdToIdmap.end())
        {
            if(m_clients.find(it->second) != m_clients.end())
            {
                Connection &conn = m_clients.at(fdToIdmap.at(event->data.fd));
                conn.m_state = SocketState::ERROR;
            }
        }
        std::unique_lock<std::mutex> lck (mtx);
        if(deleteFdFromEpollList(event->data.fd) < 0)
        {
            std::cout<<"epoll_ctl: delete failed fd : "<<event->data.fd;
        }

        close(int(event->data.fd));
    }
    else
        {
            //TODO : need to add handling for EAGAIN in read/write operations.
            //If errno == EAGAIN, that means we have read all received data from fd
            //now go for next fd.

            std::cout<<"\nhandleEvent EPOLLIN/EPOLLOUT : \n";
            if(isread)
            {
                sentData(event->data.fd);
                isread = false;
            }
            else
            {
                isread = true;
                while ( (n = read(event->data.fd, recvBuff, 1023)) > 0)
                {
                    Message *msg =(Message*)recvBuff;
                    std::cout<<"received Message Type : "<< msg->type;
                    if(msg->type == MsgType::SUBSCRIPTION_REQ)
                    {
                        std::cout<<"\nsubscription request received at fd: "<<event->data.fd;
                        //need to send ack based on the message type suported.
                    }
                    //if(msg->type == MsgType::SUBSCRIPTION_ACK)
                    //{
                    //    subscribedMessage[msg->type].insert(event->data.fd);
                    //}
                }
            }

        }
}

void ConnectionHandler::updateServerFdAndRegFd(int fd, bool add)
{
    std::unique_lock<std::mutex> lck (mtx);

    if(add)
    {
        serverFds.insert(fd);
        createAndAddFdForEpoll(uint32_t(EPOLLIN|EPOLLOUT), fd);
    }
    else
    {
        serverFds.erase(fd);
        deleteFdFromEpollList(fd);
    }
}


void ConnectionHandler::updateClientFdAndRegFd(int fd, uint16_t id, bool add)
{
    std::cout<<"\n fd : "<<fd << " id "<< id;
    std::unique_lock<std::mutex> lck (mtx);
    if(add)
    {
        fdToIdmap[fd] = id;
        clientFds.insert(fd);
        createAndAddFdForEpoll(uint32_t(EPOLLIN|EPOLLOUT), fd);
    }
    else
    {
        fdToIdmap.erase(fd);
        clientFds.erase(fd);
        deleteFdFromEpollList(fd);
    }
}


int ConnectionHandler::sendMessage(std::shared_ptr<Message> &msg)
{
    sentQueue.push(msg);
}

void ConnectionHandler::sentData(int fd)
{
    std::cout<<"\n sentData : "<<fd;
    auto it = fdToIdmap.find(fd);
    if(it != fdToIdmap.end())
    {
        //for testing purpose sending subscription message.
        if(m_clients.find(it->second) != m_clients.end())
        {
            Connection &conn = m_clients.at(fdToIdmap.at(fd));
            uint16_t len = conn.formSubscriptionMessage(senBuff);
            write(fd, senBuff, HEADER_LENGTH+len);
        }
    }
}


void ConnectionHandler::startThread()
{
    std::thread ([this]{pollForEventsOnFds();}).detach();
}
