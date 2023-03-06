#ifndef FTPSOCKET_H
#define FTPSOCKET_H


//#include <QtGlobal>
#include <QString>
#include <QList>
#include <QFile>
#include <QTextStream>

#ifdef Q_OS_WIN
    #include <WinSock2.h>
#else
    #include <cerrno>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <arpa/inet.h>
#endif

#define BLOCK_SIZE 2048

class ftpsocket
{
public:
    ftpsocket(QString remoteHost, QString remoteUser, QString remotePass);
    ~ftpsocket();
    bool Connect();
    void DisConnect();

    void Get(QString strRemoteFileName, QString strFolder, QString strLocalFileName, bool isDelete);
    QString Download(QString strRemoteFileName);
    int Upload(QString strFileName, QString code);

    QStringList GetFileList(QString path,QString strMask);
    QStringList GetDirList(QString path);
    int DeleteRemoteFile(QString strFileName);
    int ReNameFile(QString strOldFileName, QString strNewFileName);

    int MakeDir(QString strDirName);
    int RemoveDir(QString strDirName);
    void ChangeDir(QString strDirName);


private:
    QString ReadLine();
    void ReadReply();
    void SendCommand(std::string strCommand);
    bool CreateDataSocket();

#ifdef Q_OS_WIN
    SOCKET Socket_Control;
    SOCKET Socket_Data;

    SOCKADDR_IN serverAddr;
#else
    int Socket_Control;
    int Socket_Data;

    struct sockaddr_in serverAddr;
#endif
    std::string ip_addr, username, password;
    //int strRemotePort;
    int data_timeout;
    int control_timeout;
    bool bConnected;
    bool isPasv;
    QString strMsg;
    QString strReply;
    int iReplyCode;
    bool isWinCmd;

    char* buffer = new char[BLOCK_SIZE];
    char* databuffer = new char[BLOCK_SIZE];
};

#endif // FTPSOCKET_H
