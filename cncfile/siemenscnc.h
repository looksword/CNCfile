#ifndef SIEMENSCNC_H
#define SIEMENSCNC_H

#include "abstractcnc.h"

class SiemensCNC : public AbstractCNC
{
public:
    SiemensCNC(QString ip, QString user, QString pass);
    ~SiemensCNC();

    bool Connect() override;
    bool DisConnect()override;
    QStringList GetSubItemInfoOfADir(QString path)override;
    QString GetNcProgramByPath(QString path)override;
    QString GetNCDirByPath(QString path)override;
    QString GetNcDirZipByPath(QString path)override;
    bool SetNcProgramByPath(QString code,QString path)override;
    bool DelNcProgramByPath(QString path) override;
};

#endif // SIEMENSCNC_H
