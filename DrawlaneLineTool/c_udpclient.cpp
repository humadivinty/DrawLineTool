#include "c_udpclient.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>

C_UdpClient::C_UdpClient(int port):
    m_iSocket(0),
    m_iServerPort(port)
{

}

C_UdpClient::~C_UdpClient()
{
    if(m_iSocket != 0)
    {
        close(m_iSocket);
    }
}

bool C_UdpClient::ConnectToServer()
{
    struct sockaddr_in  cliaddr;
    m_iSocket = socket(AF_INET, SOCK_DGRAM, 0); /* create a socket */
    if(m_iSockID <= 0)
        return false;
    /* init servaddr */
    bzero(&cliaddr, sizeof(cliaddr));
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    cliaddr.sin_port = htons(0);
    /* bind address and port to socket */
    int iBroad = 1;
    setsockopt(m_iSocket, SOL_SOCKET, SO_BROADCAST,(void*)iBroad, sizeof(int));
    if(bind(m_iSocket, (struct sockaddr *)&cliaddr, sizeof(cliaddr)) == -1)
    {
        perror("bind error");
        return false;
    }
    return true;
}

bool C_UdpClient::SendMsgToServer(const char *SendBuff, size_t bufSize)
{
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(m_iServerPort);

    ssize_t iSendSize =  sendto(m_iSocket, SendBuff, bufSize, 0, (__CONST_SOCKADDR_ARG)&servaddr, sizeof(sockaddr_in));
    return (iSendSize == bufSize);
}
