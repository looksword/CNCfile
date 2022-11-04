#include "mitsubishicnc.h"

MitsubishiCNC::MitsubishiCNC(QString ip, QString user, QString pass):AbstractCNC(ip,user,pass)
{

}


MitsubishiCNC::~MitsubishiCNC(){

}

bool MitsubishiCNC::Connect(){

    if(!m_client.Connected())
        m_client.Connect(ip,683);

    return true;
}

bool MitsubishiCNC::DisConnect(){    
    m_client.DisConnect();
    return  true;
}

QStringList MitsubishiCNC::GetSubItemInfoOfADir(QString path){
    QString dir = "";
    m_client.File_ReadDir(path,dir);
    return dir.split(",");
}

QString MitsubishiCNC::GetNcProgramByPath(QString path){ 
    QString result;
    m_client.File_ReadFile(path,result);
    return result;
}

QString MitsubishiCNC::GetNCDirByPath(QString path){
    QString result=path;
    return result;
}

QString MitsubishiCNC::GetNcDirZipByPath(QString path){
    QString result=path;
    return result;
}

bool MitsubishiCNC::DelNcProgramByPath(QString path)
{
    return (m_client.File_DeleteFile(path) == 0) ? true:false;
}

bool MitsubishiCNC::SetNcProgramByPath(QString code,QString path){
    if(m_client.File_WriteFile(path, code) == 0)
        return true;
    return  false;
}


