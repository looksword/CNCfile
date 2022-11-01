#include "ftpsocket.h"

ftpsocket::ftpsocket(QString remoteHost, QString remoteUser, QString remotePass)
{
    this->ip_addr = remoteHost.toStdString();
    this->username = remoteUser.toStdString();
    this->password = remotePass.toStdString();
    this->isPasv = true;
#ifdef Q_OS_UNIX
    this->isWinCmd = false;
#else
    this->isWinCmd = true;
#endif
    this->bConnected = false;
    this->data_timeout = 60 * 1000;
    this->control_timeout =  30 * 1000;
    this->iReplyCode = 0;

    this->Socket_Data = 0;
}

ftpsocket::~ftpsocket()
{
#ifdef Q_OS_WIN
    closesocket(Socket_Data);
    closesocket(Socket_Control);
    WSACleanup();
#else
    close(Socket_Data);
    close(Socket_Control);
#endif
    delete[] buffer;
    delete[] databuffer;
}

bool ftpsocket::Connect()
{
    int ret;
#ifdef Q_OS_WIN
    WSADATA dat;
    //初始化
    if (WSAStartup(MAKEWORD(2,2),&dat)!=0)  //Windows Sockets Asynchronous启动
    {
        return false;
    }
#endif
    //创建Socket
    Socket_Control=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
#ifdef Q_OS_WIN
    if(Socket_Control==INVALID_SOCKET)
#else
    if(Socket_Control == -1)
#endif
    {
        return false;
    }
    //构建服务器访问参数结构体
    serverAddr.sin_family=AF_INET;
#ifdef Q_OS_WIN
    serverAddr.sin_addr.S_un.S_addr=inet_addr(ip_addr.c_str()); //地址
#else
    serverAddr.sin_addr.s_addr=inet_addr(ip_addr.c_str()); //地址
#endif
    serverAddr.sin_port=htons(21);//端口
    memset(serverAddr.sin_zero,0,sizeof(serverAddr.sin_zero));

    //连接
    ret=connect(Socket_Control,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
#ifdef Q_OS_WIN
    if(ret == SOCKET_ERROR)
#else
    if(ret == -1)
#endif
    {
        return false;
    }
#ifdef Q_OS_WIN
    Sleep(300);
#else
    struct timespec ts = {300 / 1000, (300 % 1000) * 1000 * 1000};
    nanosleep(&ts, NULL);
#endif
    ReadReply();
    if(iReplyCode != 220)
    {
        DisConnect();
        return false;
    }

    //login
    SendCommand("USER " + username);
    if(!(iReplyCode == 331 || iReplyCode == 230))
    {
        DisConnect();
        return false;
    }
    if(iReplyCode != 230)
    {
        SendCommand("PASS " +  password);
        if(!(iReplyCode == 230 || iReplyCode == 202))
        {
            DisConnect();
            return false;
        }
    }
    bConnected = true;
    return bConnected;
}

void ftpsocket::DisConnect()
{
    SendCommand("QUIT");
    ip_addr = "";
    username = "";
    password = "";
    memset(buffer, 0, BLOCK_SIZE);
    memset(databuffer, 0, BLOCK_SIZE);
#ifdef Q_OS_WIN
    closesocket(Socket_Data);
    closesocket(Socket_Control);
    WSACleanup();
#else
    close(Socket_Data);
    close(Socket_Control);
#endif
    bConnected = false;
}

void ftpsocket::ReadReply()
{
    strMsg = "";
    strReply = ReadLine();
    iReplyCode = strReply.left(3).toInt();
}

QString ftpsocket::ReadLine()
{
    memset(buffer, 0, BLOCK_SIZE);
    //::setsockopt(Socket_Control,SOL_SOCKET,SO_SNDTIMEO,(char *)&control_timeout,sizeof(control_timeout));
#ifdef Q_OS_WIN
    Sleep(50);
#else
    struct timespec ts = {50 / 1000, (50 % 1000) * 1000 * 1000};
    nanosleep(&ts, NULL);
#endif
    int recvlen = recv(Socket_Control, buffer, BLOCK_SIZE, 0);
    if(recvlen > 0)
    {
        strMsg.append(QString(buffer));
    }
    if(recvlen == BLOCK_SIZE)
    {
        strMsg = "";
    }
    //int timeout = 0;
    //::setsockopt(Socket_Control,SOL_SOCKET,SO_SNDTIMEO,(char *)&timeout,sizeof(timeout));
    QString strLine = "";
    QString seperator('\n');
    QStringList mess = strMsg.split(seperator);
    if(mess.count() > 2)
    {
        strLine = mess[mess.count() - 2];
    }
    else
    {
        strLine = mess[0];
    }
    if(strLine[3] != ' ')
    {
        return ReadLine();
    }
    return strLine;
}

void ftpsocket::SendCommand(std::string strCommand)
{
    strCommand += "\r\n";
    const char* sendcmd = strCommand.c_str();
    //设置写入超时
    //::setsockopt(Socket_Control,SOL_SOCKET,SO_SNDTIMEO,(char *)&control_timeout,sizeof(control_timeout));
#ifdef Q_OS_WIN
    send(Socket_Control, sendcmd, strlen(sendcmd), 0);
#else
    send(Socket_Control, sendcmd, strlen(sendcmd), MSG_NOSIGNAL);
#endif
    ReadReply();
}

bool ftpsocket::CreateDataSocket()
{
    if(isPasv)
    {
        SendCommand("PASV");
        if (iReplyCode != 227)
        {
            return false;
        }
        int index1 = strReply.indexOf('(');
        int index2 = strReply.indexOf(')');
        QString ipData = strReply.mid(index1 + 1, index2 - index1 - 1);
        int parts[6] = {0};
        QStringList dataparts = ipData.split(",");
        if(dataparts.count() < 6)
        {
            return false;
        }
        for(int i = 0;i < 6;i++)
        {
            parts[i] = dataparts[i].toInt();
        }
        //QString ipAddress = QString("%1.%2.%3.%4").arg(parts[0]).arg(parts[1]).arg(parts[2]).arg(parts[3]);
        int port = (parts[4] << 8) + parts[5];
        Socket_Data=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        serverAddr.sin_port=htons(port);
        int ret=connect(Socket_Data,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
#ifdef Q_OS_WIN
        if(ret==SOCKET_ERROR)
#else
        if(ret == -1)
#endif
        {
            return false;
        }
    }
    else
    {

    }

    return true;
}

void ftpsocket::Get(QString strRemoteFileName, QString strFolder, QString strLocalFileName, bool isDelete)
{
    SendCommand("TYPE I");
    if(strLocalFileName.isEmpty())
    {
        strLocalFileName = strRemoteFileName;
    }
    if(CreateDataSocket())
    {
        QFile file;
        file.setFileName(strFolder + strLocalFileName);
        if(file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream stream(&file);

            SendCommand("RETR " + strRemoteFileName.toStdString());
            if(!(iReplyCode == 150 || iReplyCode == 125
                 || iReplyCode == 226 || iReplyCode == 250))
            {
                file.close();
                return;
            }
            int recvlen = 0;
            while(true)
            {
                memset(databuffer, 0, BLOCK_SIZE);
                recvlen = recv(Socket_Data, databuffer, BLOCK_SIZE, 0);
                if(recvlen > 0)
                {
                    stream << databuffer;
                }
                else
                {
                    break;
                }
            }
            file.close();
#ifdef Q_OS_WIN
            closesocket(Socket_Data);
#else
            close(Socket_Data);
#endif
            if(!(iReplyCode == 226 || iReplyCode == 250))
            {
                ReadReply();
                if(!(iReplyCode == 226 || iReplyCode == 250))
                {
                    return;
                }
            }
            //delete remote file
            if(isDelete)
            {
                this->DeleteRemoteFile(strRemoteFileName);
            }
        }
    }
}

QString ftpsocket::Download(QString strRemoteFileName)
{
    QString code = "";
    SendCommand("TYPE I");
    if(CreateDataSocket())
    {
        SendCommand("RETR " + strRemoteFileName.toStdString());
        if(!(iReplyCode == 150 || iReplyCode == 125
             || iReplyCode == 226 || iReplyCode == 250))
        {
            return code;
        }
        int recvlen = 0;
        //::setsockopt(Socket_Data,SOL_SOCKET,SO_SNDTIMEO,(char *)&data_timeout,sizeof(data_timeout));
        while(true)
        {
            memset(databuffer, 0, BLOCK_SIZE);
            recvlen = recv(Socket_Data, databuffer, BLOCK_SIZE, 0);
            if(recvlen > 0)
            {
                code.append(QString(databuffer));
            }
            else
            {
                break;
            }
        }
#ifdef Q_OS_WIN
        closesocket(Socket_Data);
#else
        close(Socket_Data);
#endif
        if(!(iReplyCode == 226 || iReplyCode == 250))
        {
            ReadReply();
            if(!(iReplyCode == 226 || iReplyCode == 250))
            {
                return code;
            }
        }
    }

    return code;
}

int ftpsocket::Upload(QString strFileName, QString code)
{
    int iRet = 0;
    SendCommand("TYPE I");
    if(CreateDataSocket())
    {
        SendCommand("STOR " + strFileName.toStdString());
        if(!(iReplyCode == 125 || iReplyCode == 150))
        {
            return -1;
        }
        std::string strcode = std::string((const char*)code.toLatin1(), code.size());
        const char* sendcode = strcode.c_str();
#ifdef Q_OS_WIN
        send(Socket_Data, sendcode, strcode.size(), 0);
#else
        send(Socket_Data, sendcode, strcode.size(), MSG_NOSIGNAL);
#endif
#ifdef Q_OS_WIN
        closesocket(Socket_Data);
#else
        close(Socket_Data);
#endif
        if(!(iReplyCode == 226 || iReplyCode == 250))
        {
            ReadReply();
            if(!(iReplyCode == 226 || iReplyCode == 250))
            {
                return -1;
            }
        }
    }
    return iRet;
}

QStringList ftpsocket::GetFileList(QString strMask)
{
    QStringList strsFileList;
    if(CreateDataSocket())
    {
        //SendCommand("NLST " + strMask);
        //SendCommand("NLST");
        SendCommand("LIST ./");
        if(iReplyCode == 550)
        {
            return strsFileList;
        }
        if(!(iReplyCode == 150 || iReplyCode == 125 || iReplyCode == 226))
        {
            return strsFileList;
        }
        strMsg = "";
        int recvlen = 0;
        //::setsockopt(Socket_Data,SOL_SOCKET,SO_SNDTIMEO,(char *)&data_timeout,sizeof(data_timeout));
        while(true)
        {
            memset(databuffer, 0, BLOCK_SIZE);
            recvlen = recv(Socket_Data, databuffer, BLOCK_SIZE, 0);
            if(recvlen > 0)
            {
                strMsg.append(QString(databuffer));
            }
            else
            {
                break;
            }
        }
        strsFileList = strMsg.replace("\r\n","|").split("|");
#ifdef Q_OS_WIN
        closesocket(Socket_Data);
#else
        close(Socket_Data);
#endif
        if(iReplyCode != 226)
        {
            ReadReply();
            if(!(iReplyCode == 150 || iReplyCode == 226
                 || iReplyCode == 227 || iReplyCode == 426))
            {
                //return strsFileList;
                //strsFileList.clear();
            }
        }
    }
    return strsFileList;
}

QStringList ftpsocket::GetDirList()
{
    QStringList folder;
    QStringList strsFileList = GetFileList("*.*");
    if(!strsFileList.isEmpty())
    {
        foreach (QString dir, strsFileList)
        {
            if(!dir.contains("."))
            {
                folder.append(dir);
            }
        }
    }
    return folder;
}

int ftpsocket::DeleteRemoteFile(QString strFileName)
{
    int iRet = 0;
    SendCommand("DELE " + strFileName.toStdString());
    if(iReplyCode != 250)
    {
        return -1;
    }
    return iRet;
}

int ftpsocket::ReNameFile(QString strOldFileName, QString strNewFileName)
{
    int iRet = 0;
    SendCommand("RNFR " + strOldFileName.toStdString());
    if(iReplyCode != 350)
    {
        return -1;
    }
    SendCommand("RNTO " + strNewFileName.toStdString());
    if(iReplyCode != 250)
    {
        return -1;
    }
    return iRet;
}

int ftpsocket::MakeDir(QString strDirName)
{
    int iRet = 0;
    SendCommand("MKD " + strDirName.toStdString());
    if(iReplyCode != 257)
    {
        return -1;
    }
    return iRet;
}

int ftpsocket::RemoveDir(QString strDirName)
{
    int iRet = 0;
    SendCommand("RMD " + strDirName.toStdString());
    if(iReplyCode != 250)
    {
        return -1;
    }
    return iRet;
}

void ftpsocket::ChangeDir(QString strDirName)
{
    if(strDirName == "." || strDirName.isEmpty())
    {
        return;
    }
//    if(isWinCmd)
//    {
//        SendCommand("CD " + strDirName.toStdString());
//    }
//    else
//    {
        SendCommand("CWD " + strDirName.toStdString());
//    }
//    if(iReplyCode != 250)
//    {
//        ;
//    }
}

