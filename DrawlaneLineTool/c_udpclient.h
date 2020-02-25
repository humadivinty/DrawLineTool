#ifndef C_UDPCLIENT_H
#define C_UDPCLIENT_H


class C_UdpClient
{
public:
    C_UdpClient(int port);
    ~C_UdpClient();

    bool ConnectToServer();
    bool SendMsgToServer(const char* buff, size_t bufSize);
private:
    int m_iSocket;
    int m_iServerPort;
};

#endif // C_UDPCLIENT_H
