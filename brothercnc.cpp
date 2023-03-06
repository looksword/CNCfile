#include "brothercnc.h"

BrotherCNC::BrotherCNC(QString ip, QString user, QString pass):AbstractCNC(ip,user,pass){
}

BrotherCNC::~BrotherCNC(){
    delete FtpClient;
}

bool BrotherCNC::Connect(){
    FtpClient = new ftpsocket(ip,user,pass);
    return FtpClient->Connect();
}

bool BrotherCNC::DisConnect(){
    FtpClient->DisConnect();
    delete FtpClient;
    return  true;
}

QStringList BrotherCNC::GetSubItemInfoOfADir(QString path){
    QStringList result;

    if(path.isEmpty())
    {
        path = "/";
    }
    if(path.left(1) != "/")
    {
        path = "/" + path;
    }
    if(path.right(1) != "/")
    {
        path.append("/");
    }
    //FtpClient->ChangeDir(path);

    QStringList Files = FtpClient->GetFileList(path,"*.*");

    QString type = "";
    foreach(QString file,Files)
    {
        QStringList fileinfo = file.split(" ");
        if(fileinfo.count() > 1)
        {
            type = fileinfo[0].left(1);
            if(type == "-")
            {
                continue;
            }
            type = "1|" + fileinfo.last();
            result.append(type);
        }
    }
    foreach(QString file,Files)
    {
        QStringList fileinfo = file.split(" ");
        if(fileinfo.count() > 1)
        {
            type = fileinfo[0].left(1);
            if(type == "d")
            {
                continue;
            }
            type = "0|" + fileinfo.last();
            result.append(type);
        }
    }

    //FtpClient->ChangeDir("/");

    return result;
}

QString BrotherCNC::GetNcProgramByPath(QString path){
    QString result;

    result = FtpClient->Download(path);

    return result;
}

QString BrotherCNC::GetNCDirByPath(QString path){
    QString result;

    //FtpClient->ChangeDir(path);
    QStringList file = FtpClient->GetFileList(path,"*.*");
    std::string msg = "[";
    if(!file.isEmpty())
    {
        for(int i = 0;i < file.count();i++)
        {
            if(file[i].length() > 5)
            {
                msg.append("{\"path\":\"");
                QByteArray ba1 = file[i].toLatin1();
                msg.append(ba1.data());
                msg.append("\",\"content\":\"");
                QByteArray ba2 = FtpClient->Download(file[i]).toLatin1();
                msg.append(ba2.data());
                msg.append("\"}");
                if(i + 1 != file.length())
                {
                    msg.append(",");
                }
            }
        }
    }
    msg += "]";

    result = QString::fromStdString(msg);

    return result;
}

QString BrotherCNC::GetOneDirByPath(QString path, QStringList &dirs)
{
    QString result;

    if(!path.isEmpty())
    {
        FtpClient->ChangeDir(path);
    }
    else
    {
        FtpClient->ChangeDir("/");
        dirs = FtpClient->GetDirList(path);
    }
    QStringList file = FtpClient->GetFileList(path,"*.NC");
    std::string msg = "[";
    if(!file.isEmpty())
    {
        for(int i = 0;i < file.count();i++)
        {
            if(file[i].startsWith("O") && file[i].contains(".NC"))
            {
                QByteArray ba1 = (path + file[i]).toLatin1();
                msg.append(ba1.data());
                msg.append("\x01");
                QByteArray ba2 = FtpClient->Download(file[i]).toLatin1();
                msg.append(ba2.data());
                if(i + 1 != file.length())
                {
                    msg.append("\x02");
                }
            }
        }
    }

    result = QString::fromStdString(msg);

    return result;
}

QString BrotherCNC::GetNcDirZipByPath(QString path){
    QString result;

    if(!path.isEmpty())
    {
        QStringList folder;
        result.append(GetOneDirByPath("",folder));
        if(!folder.isEmpty())
        {
            foreach (QString p, folder)
            {
                if(p.length() > 0)
                {
                    QStringList dir;
                    result.append(GetOneDirByPath(p.replace("/","") + "/",dir));
                }
            }
        }
        else
        {
            QStringList dir;
            result.append(GetOneDirByPath(path.replace("/","") + "/",dir));
        }
    }

    return result;
}

bool BrotherCNC::SetNcProgramByPath(QString code,QString path){

    int iRet = FtpClient->Upload(path,code);
    if(iRet == 0)
    {
        return  true;
    }
    else
    {
        return false;
    }
}

bool BrotherCNC::DelNcProgramByPath(QString path)
{
    int iRet = FtpClient->DeleteRemoteFile(path);
    if(iRet == 0)
    {
        return  true;
    }
    else
    {
        return false;
    }
}

