#ifndef BROTHERCNC_H
#define BROTHERCNC_H

#include "abstractcnc.h"
#include "ftpsocket.h"

class BrotherCNC : public AbstractCNC
{
public:
    BrotherCNC();
    ~BrotherCNC();

    bool Connect(QString ip, QString user, QString pass) override;
    bool DisConnect()override;
    QStringList GetSubItemInfoOfADir(QString path)override;
    QString GetNcProgramByPath(QString path)override;
    QString GetNCDirByPath(QString path)override;
    QString GetNcDirZipByPath(QString path)override;
    bool SetNcProgramByPath(QString code,QString path)override;
    bool DeleteProgramByPath(QString path) override;

private:
    ftpsocket *FtpClient;
    QString GetOneDirByPath(QString path, QStringList &dirs);
};

#endif // BROTHERCNC_H
