#ifndef ABSTRACTCNC_H
#define ABSTRACTCNC_H

#include <QString>
#include <QList>

class AbstractCNC
{
public:
    AbstractCNC(QString ip, QString user, QString pass);
    virtual bool Connect()=0;
    virtual bool DisConnect()=0;
    virtual QStringList GetSubItemInfoOfADir(QString path)=0;
    virtual QString GetNcProgramByPath(QString path)=0;
    virtual QString GetNCDirByPath(QString path)=0;
    virtual QString GetNcDirZipByPath(QString path)=0;
    virtual bool SetNcProgramByPath(QString code,QString path)=0;
    virtual bool DelNcProgramByPath(QString path)=0;
protected:
    QString ip;
    QString user;
    QString pass;
};

#endif // ABSTRACTCNC_H
