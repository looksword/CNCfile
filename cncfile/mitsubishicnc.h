#ifndef MITSUBISHICNC_H
#define MITSUBISHICNC_H

#include "abstractcnc.h"
#include "ezclient.h"

class MitsubishiCNC : public AbstractCNC
{
public:
    MitsubishiCNC();
    ~MitsubishiCNC();

    bool Connect(QString ip, QString user, QString pass) override;
    bool DisConnect()override;
    QStringList GetSubItemInfoOfADir(QString path)override;
    QString GetNcProgramByPath(QString path)override;
    QString GetNCDirByPath(QString path)override;
    QString GetNcDirZipByPath(QString path)override;
    bool SetNcProgramByPath(QString code,QString path)override;
    bool DelNcProgramByPath(QString path) override;

private:
    EZClient    m_client; //< 操作类
};

#endif // MITSUBISHICNC_H
