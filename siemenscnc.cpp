#include "siemenscnc.h"

SiemensCNC::SiemensCNC(QString ip, QString user, QString pass):AbstractCNC(ip,user,pass)
{

}

SiemensCNC::~SiemensCNC(){

}

bool SiemensCNC::Connect(){
    return true;
}

bool SiemensCNC::DisConnect(){
    return  true;
}

QStringList SiemensCNC::GetSubItemInfoOfADir(QString path){
    QStringList result;
    result.append("Dir|"+path);
    return result;
}

QString SiemensCNC::GetNcProgramByPath(QString path){
    QString result=path;
    return result;
}

QString SiemensCNC::GetNCDirByPath(QString path){
    QString result=path;
    return result;
}

QString SiemensCNC::GetNcDirZipByPath(QString path){
    QString result=path;
    return result;
}

bool SiemensCNC::SetNcProgramByPath(QString code,QString path){
    code=path;
    return  true;
}

bool SiemensCNC::DelNcProgramByPath(QString path){
    path="";
    return  true;
}

