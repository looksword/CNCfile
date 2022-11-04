#include "ezclient.h"

#include <QFileInfo>

EZClient::EZClient()
{

}

int EZClient::Connect(QString IpAddress,int port)
{
    int ret = -1;
    if(_TcpClient.state() == QTcpSocket::ConnectedState)
        _TcpClient.disconnectFromHost();

    _TcpClient.connectToHost(IpAddress,port);
    _IpAddress = IpAddress;
    _Port = port;

    if(_TcpClient.waitForConnected())
    {
        InitSession();
        ret = 0;
    }
    else
    {
        ret = -1;
    }

    return ret;
}

bool EZClient::Connected()
{
    return (_TcpClient.state() == QTcpSocket::ConnectedState) ? true : false;
}

int EZClient::DisConnect()
{
    _TcpClient.disconnectFromHost();
    return 0;
}

int EZClient::InitSession()
{
    int ret = -1;
    if (_TcpClient.state() == QTcpSocket::ConnectedState)
    {
        GIOPPackage TmpPackage;
       _TcpClient.write((char*)InitArry,sizeof(InitArry));
       ret = GetPackageData(TmpPackage);
    }
    else
    {
        ret = -1;
    }

    return ret;
}

int EZClient::GetPackageData(GIOPPackage& Package)
{
    int ret = 0,MsgLen =0;
    if (_TcpClient.state() != QTcpSocket::ConnectedState)
        ret = -1;
    if (ret == 0)
        ret = GetGIOPHead(Package.GIOP_HEAD);
    if (ret == 0)
    {
        MsgLen = Package.MsgLen();
    }
    if (MsgLen > 0)
    {
        int tmplen = 0;
        tmplen = _TcpClient.read(Package.Msg.data(),MsgLen);
        if (tmplen < MsgLen)
            ret = -1;
    }
    else
        ret = -1;
    return ret;
}


int EZClient::GetGIOPHead(QByteArray& Package)
{
    int ret = 0;
    if (_TcpClient.state() != QTcpSocket::ConnectedState)
        ret = -1;

    if (ret == 0)
    {
        int tmplen = 0;
        tmplen = _TcpClient.read(Package.data(), GIOPPackage::GIOP_HEAD_LEN);
        if (tmplen < GIOPPackage::GIOP_HEAD_LEN)
            ret = -1;
        else if (Package.at(GIOPPackage::GIOP_HEAD_MSG_TYPE_INDEX) != 1)// 1 Reply
            ret = -1;
    }
    if (ret == 0)
    {
        if (Package.at(0) != 0x47 || Package.at(1) != 0x49 || Package.at(2) != 0x4f || Package.at(3) != 0x50)//GIOP
        {
            ret = -1;
            //协议不是GIOP或者数据错乱，读取所有缓存丢掉
            _TcpClient.readAll();
        }
    }
    return ret;
}

int EZClient::Device_Read(QVector<int>& ReData)
{
    int ret = -1;
    GIOPPackage TmpPackage;

    if (_TcpClient.state() == QTcpSocket::ConnectedState)
    {
        _TcpClient.write((char*)StatusArray, sizeof(StatusArray));
        ret = GetPackageData(TmpPackage);
    }
    else
        ret = -1;
    if (ret == 0 && TmpPackage.Msg.length() == 26 && TmpPackage.Msg.at(GIOPPackage::GIOP_MSG_EXCEPTION_INDEX) == 0)//head 12 msg 26  sum 38
    {
        //ReData[0] = BitConverter.ToUInt16(TmpPackage.Msg, 24);
        ReData[0] = 0;
    }

    if (_TcpClient.state() == QTcpSocket::ConnectedState)
    {
        _TcpClient.write((char*)ModeArray, sizeof(ModeArray));
        ret = GetPackageData(TmpPackage);
    }
    else
        ret = -1;
    if (ret == 0 && TmpPackage.Msg.length() == 26 && TmpPackage.Msg.at(GIOPPackage::GIOP_MSG_EXCEPTION_INDEX) == 0)//head 12 msg 26  sum 38
    {
        //ReData[1] = BitConverter.ToUInt16(TmpPackage.Msg, 24);
        ReData[1] = 1;
    }

    return ret;
}

QByteArray EZClient::GetAlarmPackageArray(int AlarmType)
{
    uint8_t bytes[] = { 0x47, 0x49, 0x4f, 0x50, 0x01, 0x00, 0x01, 0x00, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x84, 0x52, 0x00, 0x00, 0x01, 0x5d, 0xc4, 0x77, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x1d, 0x00, 0x00, 0x00, 0x6d, 0x6f, 0x63, 0x68, 0x61, 0x47, 0x65, 0x74, 0x43, 0x75, 0x72, 0x72, 0x65, 0x6e, 0x74, 0x41, 0x6c, 0x61, 0x72, 0x6d, 0x4d, 0x73, 0x67, 0x46, 0x69, 0x72, 0x73, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    int TypeIndex = 81;
    switch (AlarmType)
    {
        case EZNcDef::M_ALM_ALL_ALARM:
            bytes[TypeIndex] = 0;
            break;
        case EZNcDef::M_ALM_NC_ALARM:
            bytes[TypeIndex] = 1;
            break;
        case EZNcDef::M_ALM_STOP_CODE:
            bytes[TypeIndex] = 2;
            break;
        case EZNcDef::M_ALM_PLC_ALARM:
            bytes[TypeIndex] = 3;
            break;
        case EZNcDef::M_ALM_OPE_MSG:
            bytes[TypeIndex] = 4;
            break;
        case EZNcDef::M_ALM_WARNING:
            bytes[TypeIndex] = 5;
            break;
    }
    return QByteArray((char*)bytes,sizeof(bytes));
}

QByteArray EZClient::GIOPCmdPackge(QString operate, QString content,int mode)
{
    //head
    uint8_t head[32] = { 0x47, 0x49, 0x4f, 0x50, 0x01, 0x00, 0x01, 0x00, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x44, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01 ,0x00, 0x00, 0x00};

    //random
    int random = rand();
    head[16] = random&0xff;
    head[17] = (random>>8)&0xff;
    head[18] = (random>>16)&0xff;
    head[19] = (random>>24)&0xff;

    //cmd
    int cmd_len = operate.length() + 9;
    char* cmd = new char[cmd_len];
    memset(cmd, 0, cmd_len);

    (*(int*)cmd) = operate.length() + 1;
    strcpy(cmd+4,operate.toStdString().c_str());

    //stub
    int stub_len = 0;
    char* stub = nullptr;
    if(operate == "mochaFSStatFile")
    {
        stub_len = content.length()+4;
        stub = new char[stub_len];
        (*(int*)stub) = content.length();
        strcpy((char*)stub+4,content.toStdString().c_str());
    }
    else if(operate == "mochaFSOpenFile")
    {
        stub_len = content.length()+12;
        stub = new char[stub_len];
        head[21] = 0x3f;head[22] = 0x2a;head[23] = 0x05;
        if(mode == 1)
            (*(double*)stub) = 0;
        else
        {
            (*(double*)stub) = 0;
            stub[0] = 0x01;
            stub[1] = 0x02;
        }
        (*(int*)stub) = 8;
        strcpy((char*)stub+12,content.toStdString().c_str());
    }
    else if(operate == "mochaFSReadFile")
    {
        stub_len = 8;
        stub = new char[stub_len];
        head[21] = 0x3f;head[22] = 0x2a;head[23] = 0x05;
        (*(int*)stub) = 1;
        (*(int*)(stub+4)) = content.toInt();
    }
    else if(operate == "mochaFSWriteFile")
    {
        stub_len = content.length() + 13;
        stub = new char[stub_len];
        memset(stub, 0, stub_len);
        (*(int*)(stub+3)) = 1;
        (*(int*)(stub+7)) = content.length();
        strcpy((char*)stub+11,content.toStdString().c_str());
    }
    else if(operate == "mochaFSRemoveFile")
    {
        stub_len = content.length() + 6;
        stub = new char[stub_len];
        memset(stub, 0, stub_len);
        cmd[22] = 0xd5;cmd[23] = 0x02;
        (*(int*)(stub+2)) = content.length();
        strcpy((char*)stub+6,content.toStdString().c_str());
    }
    else if(operate == "mochaFSCloseFile")
    {
        stub_len = 7;
        stub = new char[stub_len];
        memset(stub, 0, stub_len);
        cmd[21] = 0x02;cmd[22] = 0x2a;cmd[23] = 0x05;
        (*(int*)(stub+3)) = 1;
    }
    else if(operate == "mochaFSCloseDirectory")
    {
        stub_len = 6;
        stub = new char[stub_len];
        memset(stub, 0, stub_len);
        (*(int*)(stub+3)) = 3;
    }
    else if(operate == "mochaFSOpenDirectory")
    {
        stub_len = 21;
        stub = new char[stub_len];
        memset(stub, 0, stub_len);
        (*(int*)(stub+3)) = content.length();
        strcpy((char*)stub+7,content.toStdString().c_str());
    }
    else if(operate == "mochaFSReadDirectory")
    {
        stub_len = 7;
        stub = new char[stub_len];
        memset(stub, 0, stub_len);
        (*(int*)(stub+3)) = 3;
    }

    char* bytes = new char[32 + cmd_len + stub_len];//head_len to be confirm

    //update len
    (*(int*)(head+8)) = 20 + cmd_len + stub_len;

    //packet
    memcpy(bytes, head, 32);
    memcpy(bytes+32, cmd, cmd_len);
    memcpy(bytes+32+cmd_len, stub, stub_len);

    if(cmd != nullptr)  delete []cmd;
    if(stub != nullptr) delete stub;
    if(bytes != nullptr)    delete []bytes;

    return QByteArray(bytes,32+cmd_len+stub_len);
}

int EZClient::File_ReadDir(QString path, QString &content)
{
    int ret = 0;
    bool openFlag = 0;
    GIOPPackage TmpPackage;

    QByteArray opOpenDir = GIOPCmdPackge("mochaFSOpenDirectory", path);
    if (_TcpClient.state() == QTcpSocket::ConnectedState)
    {
        _TcpClient.write(opOpenDir);
        ret = GetPackageData(TmpPackage);
    }
    else
        ret = -1;
    //stat reply
    if (ret == 0 && TmpPackage.Msg.at(GIOPPackage::GIOP_MSG_EXCEPTION_INDEX) == 0)
    {
        openFlag = true;
    }
    else
    {
        ret = -1;
    }

    if(ret == 0)
    {
        int Retry = 0;

        do
        {
            QByteArray opReadDir = GIOPCmdPackge("mochaFSReadDirectory", path);
            if (_TcpClient.state() == QTcpSocket::ConnectedState)
            {
                _TcpClient.write(opReadDir);
                ret = GetPackageData(TmpPackage);
            }
            else
                ret = -1;
            //stat reply
            if (ret == 0 && TmpPackage.Msg.at(GIOPPackage::GIOP_MSG_EXCEPTION_INDEX) == 0)
            {
                int cLen =  *((int*)(TmpPackage.Msg.data()+24)) - 1;
                if (cLen == 0) break;
                content = content + "," + QString(QByteArray(TmpPackage.Msg.data()+28,cLen));
            }
            else
            {
                if (Retry++ > 3) break;
                ret = -1;
            }
        } while (true);

        if(content.length() > 0)
            content = content.remove(0,1);
    }

    if (openFlag)
    {
        //request
        QByteArray close = GIOPCmdPackge("mochaFSCloseDirectory", "");
        if (_TcpClient.state() == QTcpSocket::ConnectedState)
        {
            _TcpClient.write(close);
            ret = GetPackageData(TmpPackage);
        }
        else
            ret = -1;
        //reply
        if (ret == 0 && TmpPackage.Msg.at(GIOPPackage::GIOP_MSG_EXCEPTION_INDEX) == 0)
        {

        }
        else
        {
            ret = -1;
        }
    }
    return ret;
}

int EZClient::File_ReadFile(QString path, QString &content)
{
    int ret = 0;
    bool openFlag = 0;
    GIOPPackage TmpPackage;

    //stat request
    QByteArray opStat = GIOPCmdPackge("mochaFSStatFile", path);
    if (_TcpClient.state() == QTcpSocket::ConnectedState)
    {
        _TcpClient.write(opStat);
        ret = GetPackageData(TmpPackage);
    }
    else
        ret = -1;
    //stat reply
    if (ret == 0 && TmpPackage.Msg.at(GIOPPackage::GIOP_MSG_EXCEPTION_INDEX) == 0)
    {

    }
    else
    {
        ret = -1;
    }

    //open
    if (ret == 0)
    {
        //request
        QByteArray opOpen = GIOPCmdPackge("mochaFSOpenFile", path);
        if (_TcpClient.state() == QTcpSocket::ConnectedState)
        {
            _TcpClient.write(opOpen);
            ret = GetPackageData(TmpPackage);
        }
        else
            ret = -1;
        //reply
        if (ret == 0 && TmpPackage.Msg.at(GIOPPackage::GIOP_MSG_EXCEPTION_INDEX) == 0)
        {
            openFlag = true;
        }
        else
        {
            ret = -1;
        }
    }

    //read
    if (ret == 0 && openFlag)
    {
        int Retry = 0;
        do
        {
            //request
            QByteArray opRead = GIOPCmdPackge("mochaFSReadFile", "500");
            if (_TcpClient.state() == QTcpSocket::ConnectedState)
            {
                _TcpClient.write(opRead);
                ret = GetPackageData(TmpPackage);
            }
            else
                ret = -1;
            //reply
            if (ret == 0 && TmpPackage.Msg.at(GIOPPackage::GIOP_MSG_EXCEPTION_INDEX) == 0)
            {
                content += QString(QByteArray(TmpPackage.Msg.data()+20,TmpPackage.Msg.length()-20));
                if(TmpPackage.Msg.length() < 500)
                {
                    break;
                }
            }
            else
            {
                if (Retry++ >= 3) break;
                ret = -1;
            }
        }while(true);
    }

    if (openFlag)
    {
        //request
        QByteArray close = GIOPCmdPackge("mochaFSCloseDirectory", "");
        if (_TcpClient.state() == QTcpSocket::ConnectedState)
        {
            _TcpClient.write(close);
            ret = GetPackageData(TmpPackage);
        }
        else
            ret = -1;
        //reply
        if (ret == 0 && TmpPackage.Msg.at(GIOPPackage::GIOP_MSG_EXCEPTION_INDEX) == 0)
        {

        }
        else
        {
            ret = -1;
        }
    }
    return ret;
}

int EZClient::File_WriteFile(QString path, QString &content)
{
    int ret = 0;
    bool openFlag = 0;
    GIOPPackage TmpPackage;

    //stat dir
    QByteArray stat = GIOPCmdPackge("mochaFSStatFile", QFileInfo(path).fileName());
    if (_TcpClient.state() == QTcpSocket::ConnectedState)
    {
        _TcpClient.write(stat);
        ret = GetPackageData(TmpPackage);
    }
    else
        ret = -1;
    //stat reply
    if (ret == 0 && TmpPackage.Msg.at(GIOPPackage::GIOP_MSG_EXCEPTION_INDEX) == 0)
    {

    }
    else
    {
        ret = -1;
    }

    //stat file
    QByteArray statdir = GIOPCmdPackge("mochaFSStatFile", path);
    if (_TcpClient.state() == QTcpSocket::ConnectedState)
    {
        _TcpClient.write(statdir);
        ret = GetPackageData(TmpPackage);
    }
    else
        ret = -1;
    //stat reply
    if (ret == 0 && TmpPackage.Msg.at(GIOPPackage::GIOP_MSG_EXCEPTION_INDEX) == 0)
    {
        // delete
        QByteArray del = GIOPCmdPackge("mochaFSRemoveFile", path);
        if (_TcpClient.state() == QTcpSocket::ConnectedState)
        {
            _TcpClient.write(del);
            ret = GetPackageData(TmpPackage);
        }
        else
            ret = -1;
        //stat reply
        if (ret == 0 && TmpPackage.Msg.at(GIOPPackage::GIOP_MSG_EXCEPTION_INDEX) == 0)
        {

        }
        else
        {
            ret = -1;
        }
    }

    //open
    if (ret == 0)
    {
        //request
        QByteArray open = GIOPCmdPackge("mochaFSOpenFile", path, 2);
        if (_TcpClient.state() == QTcpSocket::ConnectedState)
        {
            _TcpClient.write(open);
            ret = GetPackageData(TmpPackage);
        }
        else
            ret = -1;
        //reply
        if (ret == 0 && TmpPackage.Msg.at(GIOPPackage::GIOP_MSG_EXCEPTION_INDEX) == 0)
        {
            openFlag = true;
        }
        else
        {
            ret = -1;
        }
    }

    //read
    if (ret == 0 && content.length() > 0)
    {
        int Retry = 0;
        for (int i = 0; i < content.length();)
        {
            int len = 500;
            if (content.length() - i > 500)
                len = 500;
            else
                len = content.length() - i;
            //request
            QByteArray write = GIOPCmdPackge("mochaFSWriteFile", QString(content.toStdString().substr(i,len).c_str()));
            if (_TcpClient.state() == QTcpSocket::ConnectedState)
            {
                _TcpClient.write(write);
                ret = GetPackageData(TmpPackage);
            }
            else
                ret = -1;
            //reply
            if (ret == 0 && TmpPackage.Msg.at(GIOPPackage::GIOP_MSG_EXCEPTION_INDEX) == 0)
            {
                i+=len;
            }
            else
            {
                if (Retry++ >= 3) break;
                ret = -1;
            }
        }
    }

    if (openFlag)
    {
        //request
        QByteArray close = GIOPCmdPackge("mochaFSCloseDirectory", "");
        if (_TcpClient.state() == QTcpSocket::ConnectedState)
        {
            _TcpClient.write(close);
            ret = GetPackageData(TmpPackage);
        }
        else
            ret = -1;
        //reply
        if (ret == 0 && TmpPackage.Msg.at(GIOPPackage::GIOP_MSG_EXCEPTION_INDEX) == 0)
        {

        }
        else
        {
            ret = -1;
        }
    }
    return ret;
}

int EZClient::File_DeleteFile(QString &path)
{
    int ret = 0;
    GIOPPackage TmpPackage;

    if (ret == 0)
    {
        int Retry = 0;
        do
        {
            //request
            QByteArray del = GIOPCmdPackge("mochaFSRemoveFile", path);
            if (_TcpClient.state() == QTcpSocket::ConnectedState)
            {
                _TcpClient.write(del);
                ret = GetPackageData(TmpPackage);
            }
            else
                ret = -1;
            //reply
            if (ret == 0 && TmpPackage.Msg.at(GIOPPackage::GIOP_MSG_EXCEPTION_INDEX) == 0)
            {
                break;
            }
            else
            {
                if (Retry++ >= 3) break;
                ret = -1;
            }
        } while (true);
    }
    return ret;
}

int EZClient::CommonVariable_Read2(int index,double& value)
{
    int ret = 0;
    value = -1;
    char bytes[] = { 0x47, 0x49, 0x4f, 0x50, 0x01, 0x00, 0x01, 0x00, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x54, 0x1e, 0x00, 0x00, 0x01, 0x00, 0x06, 0x0a, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x6d, 0x6f, 0x63, 0x68, 0x61, 0x47, 0x65, 0x74, 0x44, 0x61, 0x74, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x39, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00 };
    GIOPPackage TmpPackage;
    int tmpIndex = index + 900;
    bytes[60] = tmpIndex&0xff;
    bytes[61] = (tmpIndex>>8)&0xff;

    if (_TcpClient.state() == QTcpSocket::ConnectedState)
    {
        _TcpClient.write(bytes,sizeof(bytes));
        ret = GetPackageData(TmpPackage);
    }
    else
        ret = -1;
    if (ret == 0 && TmpPackage.Msg.length() == 40 && TmpPackage.Msg.at(GIOPPackage::GIOP_MSG_EXCEPTION_INDEX) == 0)
    {
        value = *((double*)TmpPackage.Msg.data()+32);
    }
    else
        ret = -1;

    return ret;
}

int EZClient::CommonVariable_Read2(int index, double& value, int RetryTimes)
{
    int ret = 0;
    do
    {
        ret = CommonVariable_Read2(index, value);
        if (ret == 0)
            break;
    }
    while (RetryTimes-- >= 0);
    return ret;
}

int EZClient::CommonVariable_Write2(int index, double value)
{
    int ret = 0;
    GIOPPackage TmpPackage;
    unsigned char bytes[] = { 0x47,0x49,0x4f,0x50,0x01,0x00,0x01,0x00,0x50,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7c,0x49,0x00,0x00,0x01,0xf9,0x0b,0x01,0x04,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x0d,0x00,0x00,0x00,0x6d,0x6f,0x63,0x68,0x61,0x53,0x65,0x74,0x44,0x61,0x74,0x61,0x00,0x76,0x3d,0x07,0x00,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0xf5,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
    int tmpIndex = index + 900;
    bytes[60] = tmpIndex&0xff;
    bytes[61] = (tmpIndex>>8)&0xff;

    *(double*)(bytes+84) = value;

    if (_TcpClient.state() == QTcpSocket::ConnectedState)
    {
        _TcpClient.write((char*)bytes, sizeof(bytes));
        ret = GetPackageData(TmpPackage);
    }
    else
        ret = -1;
    if (ret == 0 && TmpPackage.Msg.length() == 16 && TmpPackage.Msg.at(GIOPPackage::GIOP_MSG_EXCEPTION_INDEX) == 0)
    {
        ;
    }
    else
        ret = -1;

    return ret;
}


int EZClient::ATC_GetMGNReady2(int lMagazineNo, int lReady, int &plToolNo)
{
    int ret = lMagazineNo;
    ret = 0;
    plToolNo = 0;
    unsigned char bytes[] = { 0x47, 0x49, 0x4f, 0x50, 0x01, 0x00, 0x01, 0x00, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb0, 0x37, 0x00, 0x00, 0x01, 0x5d, 0xc4, 0x77, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x6d, 0x6f, 0x63, 0x68, 0x61, 0x47, 0x65, 0x74, 0x44, 0x61, 0x74, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x37, 0x00, 0x00, 0x00, 0x1c, 0xb0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00 };
    GIOPPackage TmpPackage;
    int ReadyIndex = 60;
    switch (lReady)
    {
        case 0:
            bytes[ReadyIndex] = 0x1C;
            break;
        case 1:
            bytes[ReadyIndex] = 0x1D;
            break;
        case 2:
            bytes[ReadyIndex] = 0x1F;
            break;
        case 3:
            bytes[ReadyIndex] = 0x20;
            break;
        case 4:
            bytes[ReadyIndex] = 0x21;
            break;
        default:
            ret = -1;
            break;
    }
    if (ret == 0)
    {
        if (_TcpClient.state() == QTcpSocket::ConnectedState)
        {
            _TcpClient.write((char*)bytes, sizeof(bytes));
            ret = GetPackageData(TmpPackage);
        }
        else
            ret = -1;
        if (ret == 0 && TmpPackage.Msg.length() == 26 && TmpPackage.Msg.at(GIOPPackage::GIOP_MSG_EXCEPTION_INDEX) == 0)
        {
            int Uints_digit = TmpPackage.Msg[TmpPackage.Msg.length() - 2] % 16;//个位
            int Uints_tens = TmpPackage.Msg[TmpPackage.Msg.length() - 2] / 16;//十位
            plToolNo = Uints_tens * 10 + Uints_digit;
        }
    }
    return ret;
}


int EZClient::ATC_GetMGNReady2(int lMagazineNo, int lReady, int& plToolNo, int RetryTimes)
{
    int ret = 0;
    do
    {
        ret = ATC_GetMGNReady2(lMagazineNo,lReady,plToolNo);
        if (ret == 0)
            break;
    }
    while (RetryTimes-- >= 0);
    return ret;
}

int EZClient::WriteDevice(uint Address,int DataType,int DataValue)
{
    int ret = 0;
    int bytes_len = 0;
    unsigned char* bytes=nullptr;
    GIOPPackage TmpPackage;
    switch(DataType)
    {//数值
        case EZNcDef::EZNC_PLC_BIT:
        case EZNcDef::EZNC_PLC_BYTE:
            bytes_len = 85;
            bytes = new unsigned char[bytes_len] { 0x47, 0x49, 0x4f, 0x50, 0x01, 0x00, 0x01, 0x00, 0x49, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8c, 0x1c, 0x00, 0x00, 0x01, 0x00, 0xf1, 0x02, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x6d, 0x6f, 0x63, 0x68, 0x61, 0x53, 0x65, 0x74, 0x44, 0x61, 0x74, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x35, 0x00, 0x00, 0x00, 0x28, 0xa0, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00 };
            bytes[bytes_len - 1] = DataValue&0xff;
            break;
        case EZNcDef::EZNC_PLC_WORD:
            bytes_len = 86;
            bytes = new unsigned char[bytes_len] { 0x47, 0x49, 0x4f, 0x50, 0x01, 0x00, 0x01, 0x00, 0x4a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8c, 0x1c, 0x00, 0x00, 0x01, 0x00, 0xf1, 0x02, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x6d, 0x6f, 0x63, 0x68, 0x61, 0x53, 0x65, 0x74, 0x44, 0x61, 0x74, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x37, 0x00, 0x00, 0x00, 0x28, 0xa0, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00 };
            bytes[bytes_len - 1] = (DataValue>>8)&0xff;
            bytes[bytes_len - 2] = DataValue&0xff;
            break;
        case EZNcDef::EZNC_PLC_DWORD:
            bytes_len = 87;
            bytes = new unsigned char[bytes_len] { 0x47, 0x49, 0x4f, 0x50, 0x01, 0x00, 0x01, 0x00, 0x4c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8c, 0x1c, 0x00, 0x00, 0x01, 0x00, 0xf1, 0x02, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x6d, 0x6f, 0x63, 0x68, 0x61, 0x53, 0x65, 0x74, 0x44, 0x61, 0x74, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x28, 0xa0, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            bytes[bytes_len - 1] = (DataValue>>24)&0xff;
            bytes[bytes_len - 2] = (DataValue>>16)&0xff;
            bytes[bytes_len - 3] = (DataValue>>8)&0xff;
            bytes[bytes_len - 4] = DataValue&0xff;
            break;
        default:
            ret = -1;
            break;
    }
    if (ret == 0)
    {//地址
        *(int*)(bytes+60) = Address;
    }
    if (ret == 0)
    {
        if (_TcpClient.state() == QTcpSocket::ConnectedState)
        {
            _TcpClient.write((char*)bytes, bytes_len);
            ret = GetPackageData(TmpPackage);
        }
        else
            ret = -1;
    }
    return ret;
}

int EZClient::GetDeivceADD(QString DeviceName,uint& Address)
{
    int ret = 0;
    QString tmpstr = DeviceName.trimmed();
    tmpstr = tmpstr.replace(" ", "");
    tmpstr = tmpstr.toUpper();
    if (tmpstr.startsWith("F"))//F
    {
        tmpstr = tmpstr.toStdString().substr(1).c_str();
        uint tmpint = tmpstr.toUInt();
        if (tmpint)//十进制名称 例如1024
        {
            EZDevice dev;
            Address = tmpint + dev.DEVICE_F_ADD0[0];//基于0地址偏移
        }
        else
            ret = -1;
    }
    else //其他 类型Device有16进制名称  暂空
        ret = -1;
    return ret;
}

int EZClient::System_GetAlarm(QString& AlarmMsg)
{
    int ret = 0;
    GIOPPackage TmpPackage;
    QByteArray bytes = GetAlarmPackageArray(EZNcDef::M_ALM_ALL_ALARM);

    if (_TcpClient.state() == QTcpSocket::ConnectedState)
    {
        _TcpClient.write(bytes);
        ret = GetPackageData(TmpPackage);
    }
    else
        ret = -1;
    if (ret == 0 && TmpPackage.Msg.length() > 23 && TmpPackage.Msg.at(GIOPPackage::GIOP_MSG_EXCEPTION_INDEX) == 0)
    {
        //System.Text.Encoding ed = System.Text.Encoding.GetEncoding("GB2312");
        //AlarmMsg = ed.GetString(TmpPackage.Msg, 20, TmpPackage.Msg.Length - 23);
        AlarmMsg = QByteArray(TmpPackage.Msg.data()+20,TmpPackage.Msg.length() - 23);
    }

    return ret;
}

int EZClient::System_GetAlarm(QString& AlarmMsg, int RetryTimes)
{
    int ret = 0;
    do
    {
        ret = System_GetAlarm(AlarmMsg);
        if (ret == 0)
            break;
    }
    while (RetryTimes-- >= 0);
    return ret;
}

int EZClient::SetAlarm(QString DeviceName)
{
    int ret = 0;
    uint TmpAdd = 0;
    ret = GetDeivceADD(DeviceName, TmpAdd);
    if(ret==0)
    {
        WriteDevice(TmpAdd, EZNcDef::EZNC_PLC_BIT, 1);
    }
    return ret;
}

int EZClient::ClearAlarm(QString DeviceName)
{
    int ret = 0;
    uint TmpAdd = 0;
    ret = GetDeivceADD(DeviceName,TmpAdd);
    if (ret == 0)
    {
        WriteDevice(TmpAdd, EZNcDef::EZNC_PLC_BIT, 0);
    }
    return ret;
}

int EZClient::Tool_SetOffset(int tid, float x0, float x1, float x2, float x3)
{
    int ret = 0;
    GIOPPackage TmpPackage;
    unsigned char bytes[] = { 0x47, 0x49, 0x4f, 0x50, 0x01, 0x00, 0x01, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa8, 0x56, 0x00, 0x00, 0x01, 0x04, 0x10, 0x05, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x6d, 0x6f, 0x63, 0x68, 0x61, 0x53, 0x65, 0x74, 0x44, 0x61, 0x74, 0x61, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0xb9, 0x0b, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x02, 0x40 };

    if (_TcpClient.state() == QTcpSocket::ConnectedState)
    {
        //x0
        if(x0 != 99999)
        {
            bytes[60] = tid&0xff;
            bytes[61] = (tid>>8)&0xff;

            *(double*)(bytes+84) = x0;

            _TcpClient.write((char*)bytes, sizeof(bytes));
            ret = GetPackageData(TmpPackage);
        }


        //x1
        if (x1 != 99999)
        {
            bytes[60] = (tid+2000)&0xff;
            bytes[61] = ((tid+2000)>>8)&0xff;

            *(double*)(bytes+84) = x1;

            _TcpClient.write((char*)bytes, sizeof(bytes));
            ret = GetPackageData(TmpPackage);
        }


        //x2
        if (x2 != 99999)
        {
            bytes[60] = (tid+1000)&0xff;
            bytes[61] = ((tid+1000)>>8)&0xff;

            *(double*)(bytes+84) = x2;

            _TcpClient.write((char*)bytes, sizeof(bytes));
            ret = GetPackageData(TmpPackage);
        }


        //x3
        if (x3 != 99999)
        {
            bytes[60] = (tid+3000)&0xff;
            bytes[61] = ((tid+3000)>>8)&0xff;

            *(double*)(bytes+84) = x3;

            _TcpClient.write((char*)bytes, sizeof(bytes));
            ret = GetPackageData(TmpPackage);
        }
    }
    else
        ret = -1;
    if (ret == 0 && TmpPackage.Msg.length() == 16 && TmpPackage.Msg.at(GIOPPackage::GIOP_MSG_EXCEPTION_INDEX) == 0)
    {
        ;
    }
    else
        ret = -1;

    return ret;
}
