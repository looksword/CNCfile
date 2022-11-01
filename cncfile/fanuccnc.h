#ifndef FANUCCNC_H
#define FANUCCNC_H

#include "abstractcnc.h"
#include "Fwlib32.h"

class FanucCNC : public AbstractCNC
{
public:
    FanucCNC();
    ~FanucCNC();

    bool Connect(QString ip, QString user, QString pass) override;
    bool DisConnect()override;
    QStringList GetSubItemInfoOfADir(QString path)override;
    QString GetNcProgramByPath(QString path)override;
    QString GetNCDirByPath(QString path)override;
    QString GetNcDirZipByPath(QString path)override;
    bool SetNcProgramByPath(QString code,QString path)override;
    bool DeleteProgramByPath(QString path) override;

private:
    char* ToStdPrgName(char* path,char* out);
    std::string GetNcProgramByPathEx(char* path, int* errCode);
    int GetNcFile(const char *path, std::string &msg, int mode);
    int GetDirOrFile(const char *path, std::string &msg, int mode);
    std::string GetRandomMainProgram();

    unsigned short h;
};

#endif // FANUCCNC_H
