#include "siemenscnc.h"

SiemensCNC::SiemensCNC()
{

}

SiemensCNC::~SiemensCNC(){

}

bool SiemensCNC::Connect(QString ip, QString user, QString pass){
    return true;
}

bool SiemensCNC::DisConnect(){
    return  true;
}

QStringList SiemensCNC::GetSubItemInfoOfADir(QString path){
    QStringList result;
    return result;
}

QString SiemensCNC::GetNcProgramByPath(QString path){
    QString result;
    return result;
}

QString SiemensCNC::GetNCDirByPath(QString path){
    QString result;
    return result;
}

QString SiemensCNC::GetNcDirZipByPath(QString path){
    QString result;
    return result;
}

bool SiemensCNC::SetNcProgramByPath(QString code,QString path){
    return  true;
}

bool SiemensCNC::DeleteProgramByPath(QString path){
    return  true;
}

