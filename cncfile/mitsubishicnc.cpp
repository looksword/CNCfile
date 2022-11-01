#include "mitsubishicnc.h"

MitsubishiCNC::MitsubishiCNC()
{

}


MitsubishiCNC::~MitsubishiCNC(){

}

bool MitsubishiCNC::Connect(QString ip, QString user, QString pass){
    return true;
}

bool MitsubishiCNC::DisConnect(){
    return  true;
}

QStringList MitsubishiCNC::GetSubItemInfoOfADir(QString path){
    QStringList result;
    return result;
}

QString MitsubishiCNC::GetNcProgramByPath(QString path){
    QString result;
    return result;
}

QString MitsubishiCNC::GetNCDirByPath(QString path){
    QString result;
    return result;
}

QString MitsubishiCNC::GetNcDirZipByPath(QString path){
    QString result;
    return result;
}

bool MitsubishiCNC::SetNcProgramByPath(QString code,QString path){
    return  true;
}

bool MitsubishiCNC::DeleteProgramByPath(QString path){
    return  true;
}

