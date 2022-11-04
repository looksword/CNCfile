#include "fanuccnc.h"

FanucCNC::FanucCNC(QString ip, QString user, QString pass):AbstractCNC(ip,user,pass)
{
#ifdef Q_OS_UNIX
    cnc_startupprocess(3,"fwlibeth.log");
#endif
    this->OldPath = "//CNC_MEM/";
}

FanucCNC::~FanucCNC(){

}

bool FanucCNC::Connect(){
    char* tempip;
    QByteArray newip = ip.toLatin1();
    tempip = newip.data();
    h = 0;
    short ret = EW_OK;
    ret = cnc_allclibhndl3(tempip,8193,3,&h);
    if(ret != EW_OK){
        return false;
    }
    return true;
}

bool FanucCNC::DisConnect(){
    cnc_freelibhndl(h);
    return  true;
}

char* FanucCNC::ToStdPrgName(char* path,char* out)
{
    if ((path[0] == 'O') && (strlen(path) <= 5))
    {
        int prgnum = atoi(path + 1);
        sprintf(out, "%c%04d", path[0], prgnum);

    }
    else
    {
        return path;
    }
    return out;
}

QStringList FanucCNC::GetSubItemInfoOfADir(QString path){
    QStringList result;
    if(path.isEmpty())
    {
        path = OldPath;
    }
    else
    {
        if(!path.contains("//CNC_MEM/"))
        {
            path = OldPath + path + "/";
        }
        OldPath = path;
    }
    std::string pathStr = path.toStdString();
    if(pathStr[pathStr.length() - 1] != '/')
    {
        pathStr.append("/");
    }
    char* temppath = (char*)pathStr.c_str();

    short ret = EW_OK;
    short num = 0;
    short count = 0;

    IDBPDFSDIR idbPdfSDir;
    ODBPDFSDIR odbPdfSDirs[100];
    memset(&idbPdfSDir, 0, sizeof(idbPdfSDir));
    strcpy(idbPdfSDir.path, temppath);
    while(true)
    {
        num = 100;
        idbPdfSDir.req_num = count;
        memset(odbPdfSDirs, 0, sizeof(odbPdfSDirs));
        ret = cnc_rdpdf_subdir(h,&num,&idbPdfSDir, odbPdfSDirs);

        count += num;
        for(int i = 0;i < num; i++)
        {
            QString newstr(odbPdfSDirs[i].d_f);
            result.append("Dir|" + newstr);
        }
        if (num < 100)
        {
            break;
        }
    }

    IDBPDFADIR idbPath;
    ODBPDFADIR prg[100];
    count = 0;
    memset(&idbPath, 0, sizeof(IDBPDFADIR));
    strcpy(idbPath.path, temppath);
    int fileCount = 0;
    while(true)
    {
        num = 100;
        idbPath.req_num = count;
        idbPath.size_kind = 1;
        idbPath.type = 0;
        memset(prg, 0, sizeof(prg));
        ret = cnc_rdpdf_alldir(h,&num,&idbPath,prg);
        if(ret == EW_NUMBER)
        {
            break;
        }

        count += num;
        for (int i = 0; i < num; i++)
        {
            char prgn[10] = {0};
            if (prg[i].data_kind == 0)
            {
                continue;
            }
            fileCount++;

            QString newstr(ToStdPrgName(prg[i].d_f,prgn));
            result.append("File|" + newstr);
        }
        if (num < 100)
        {
            break;
        }
    }

    delete temppath;

    return result;
}

QString FanucCNC::GetNcProgramByPath(QString path){
    QString result;

    char* temppath;
    QByteArray ba = path.toLatin1();
    temppath = ba.data();

    char buf[1281];
    long count = 0;
    short ret = 0;

    for (int i = 0; i < 100;i++)
    {
        ret = cnc_upstart4(h,0,temppath);
        if(ret != EW_OK)
        {
            if(i >= 99)
            {
                cnc_upend4(h);
#ifdef Q_OS_WIN
                Sleep(50);
#else
                struct timespec ts = {50 / 1000, (50 % 1000) * 1000 * 1000};
                nanosleep(&ts, NULL);
#endif
                return result;
            }
            if(ret == EW_BUSY || ret == EW_RESET)
            {
                cnc_upend4(h);
#ifdef Q_OS_WIN
                Sleep(50);
#else
                struct timespec ts = {50 / 1000, (50 % 1000) * 1000 * 1000};
                nanosleep(&ts, NULL);
#endif
            }
        }
        if (ret == EW_OK)
        {
            break;
        }
#ifdef Q_OS_WIN
        Sleep(100);
#else
        struct timespec ts = {100 / 1000, (100 % 1000) * 1000 * 1000};
        nanosleep(&ts, NULL);
#endif
    }

    std::string prg = "";
    int retryCount = 0;
    while(true)
    {
        count = sizeof(buf) - 1;
        memset(buf, 0, sizeof(buf));
        ret = cnc_upload4(h, &count, buf);
        if (ret == EW_BUFFER)
        {
#ifdef Q_OS_WIN
            Sleep(8);
#else
            struct timespec ts = {8 / 1000, (8 % 1000) * 1000 * 1000};
            nanosleep(&ts, NULL);
#endif
            retryCount++;
            if(retryCount < 1000)
            {
                continue;
            }
        }
        if (ret != EW_OK)
        {
            break;
        }
        retryCount = 0;
        if ((count > 0) && (count <= (int)sizeof(buf) - 1))
        {
            buf[count] = 0;
            prg.append(buf);
        }
    }

    for (int i = 0; i < 100;i++)
    {
        ret = cnc_upend4(h);
        if(ret != EW_OK)
        {
            if(i >= 99)
            {
                return result;
            }
        }
        if (ret == EW_OK)
        {
            break;
        }
#ifdef Q_OS_WIN
        Sleep(100);
#else
        struct timespec ts = {100 / 1000, (100 % 1000) * 1000 * 1000};
        nanosleep(&ts, NULL);
#endif
    }

    delete temppath;

    result = QString::fromStdString(prg);

    return result;
}

std::string FanucCNC::GetNcProgramByPathEx(char* path, int* errCode)
{
    char buf[1281];
    long count = 0;
    short ret = 0;

    for (int i = 0; i < 100; i++)
    {
        ret = cnc_upstart4(h, 0, path);
        if (ret != EW_OK)
        {
#ifdef Q_OS_WIN
            Sleep(100);
#else
            struct timespec ts = {100 / 1000, (100 % 1000) * 1000 * 1000};
            nanosleep(&ts, NULL);
#endif
            if (i >= 99)
            {
                *errCode = ret;
                cnc_upend4(h);
#ifdef Q_OS_WIN
            Sleep(50);
#else
            struct timespec ts2 = {50 / 1000, (50 % 1000) * 1000 * 1000};
            nanosleep(&ts2, NULL);
#endif
                return "";
            }

            if (ret == EW_BUSY)
            {
                cnc_upend4(h);
#ifdef Q_OS_WIN
                Sleep(50);
#else
                struct timespec ts3 = {50 / 1000, (50 % 1000) * 1000 * 1000};
                nanosleep(&ts3, NULL);
#endif
            }
        }

        if (ret == EW_OK)
            break;

#ifdef Q_OS_WIN
        Sleep(100);
#else
        struct timespec ts = {100 / 1000, (100 % 1000) * 1000 * 1000};
        nanosleep(&ts, NULL);
#endif
    }

    std::string msg = "";
    int retryCount = 0;
    while (true)
    {
        count = sizeof(buf)-1;
        memset(buf, 0, sizeof(buf));
        ret = cnc_upload4(h, &count, buf);
        if (ret == EW_BUFFER)
        {
#ifdef Q_OS_WIN
            Sleep(8);
#else
            struct timespec ts = {8 / 1000, (8 % 1000) * 1000 * 1000};
            nanosleep(&ts, NULL);
#endif
            retryCount++;
            if (retryCount < 1000)
            {
                continue;
            }
        }
        if (ret != EW_OK)
        {
            break;
        }
        retryCount = 0;
        if ((count > 0) && (count <= (int)sizeof(buf)-1))
        {
            buf[count] = 0;
            msg.append(buf);
        }
    }

    for (int i = 0; i < 10; i++)
    {
        ret = cnc_upend4(h);
        if (ret != EW_OK)
        {
            if (i >= 9)
            {
                *errCode = ret;
                return "";
            }
        }

        if (ret == EW_OK)
            break;

#ifdef Q_OS_WIN
        Sleep(100);
#else
        struct timespec ts = {100 / 1000, (100 % 1000) * 1000 * 1000};
        nanosleep(&ts, NULL);
#endif
    }

    if (msg.empty())
    {
        *errCode = -18;
        return "";
    }

    return msg;
}

int FanucCNC::GetNcFile(const char *path, std::string &msg, int mode)
{
    int errCode = 0;
    std::string szPath = path;
    std::string code = "";

    for (int i = 0; i < 3; i++)
    {
        code = GetNcProgramByPathEx((char*)path, &errCode);
        if ( !code.empty() )
        {
            break;
        }
    }

    if (code.empty() || errCode < 0)
    {
        return errCode;
    }

    //拼装
    if (mode == 1)
    {
        //替换"
        int pos = 0;
        pos = code.find("\"");
        while (pos != -1)
        {
            code.insert(pos, "\\");
            pos = code.find("\"",pos + 2);
        }

        msg += "{\"path\":\"" + szPath.substr(10, szPath.length()) + "\",\"content\":\"" + code + "\"},";
    }
    else
    {
        msg += szPath.substr(10, szPath.length()) + "\x01" + code + "\x02";
    }

    return 0;
}

int FanucCNC::GetDirOrFile(const char *path, std::string &msg, int mode)
{
    short num = 100;
    IDBPDFADIR idbPath;
    ODBPDFADIR prg[100];
    memset(&idbPath, 0, sizeof(IDBPDFADIR));
    strcpy(idbPath.path, path);

    idbPath.req_num = 0;
    idbPath.size_kind = 1;
    idbPath.type = 0;
    memset(prg,0,sizeof(prg));
    int ret = cnc_rdpdf_alldir(h,&num,&idbPath,prg);
    if(ret != EW_OK)
    {
        std::string szPath = path;
        if(mode == 1)
        {
            msg += "\"path\":\"" + szPath.substr(10,szPath.length()) + "\",\"content\":\"\"},";
        }
        else
        {
            msg += szPath.substr(10, szPath.length()) + "\x01" + "\x02";
        }
        return 0;
    }
    for (int i = 0; i < num;i++)
    {
        int iRet = 0;
        std::string subpath = path;
        //dir
        if(prg[i].data_kind == 0)
        {
            GetDirOrFile(subpath.append(prg[i].d_f).append("/").c_str(),msg,mode);
        }
        else
        {
            char prgn[10] = {0};
            iRet = GetNcFile(subpath.append(ToStdPrgName(prg[i].d_f,prgn)).c_str(),msg,mode);
            if(iRet < 0)
            {
                return iRet;
            }
        }
    }
    return 0;
}

std::string FanucCNC::GetRandomMainProgram()
{
    IDBPDFADIR idbPath;
    ODBPDFADIR prg[10];
    memset(&idbPath, 0, sizeof(IDBPDFADIR));
    strcpy(idbPath.path, "//CNC_MEM/SYSTEM/");

    short num = 10;
    std::string path = "//CNC_MEM/SYSTEM/";

    while (true)
    {
        idbPath.req_num = 0;
        idbPath.size_kind = 1;
        idbPath.type = 0;
        memset(prg, 0, sizeof(prg));
        short ret = cnc_rdpdf_alldir(h, &num, &idbPath, prg);
        if (ret == EW_NUMBER)
        {
            break;
        }
        if (ret == EW_OK)
        {
            for (int i = 0; i < num; i++)
            {
                if (prg[i].data_kind == 0)
                {
                    continue;
                }

                return (path + prg[i].d_f);
            }
        }
    }

    return "//CNC_MEM/SYSTEM/O0123";
}

QString FanucCNC::GetNCDirByPath(QString path){
    QString result;

    int iRet = 0;
    std::string msg = "[";
    char* temppath;
    QByteArray ba = path.toLatin1();
    temppath = ba.data();
    iRet = GetDirOrFile(temppath/*"//CNC_MEM/USER/"*/, msg, 1);
    if (iRet < 0)
    {
        return result;
    }
    if (msg.rfind(",") > 0)
    {
        msg.erase(msg.rfind(","));
    }
    msg += "]";

    result = QString::fromStdString(msg);

    return result;
}

QString FanucCNC::GetNcDirZipByPath(QString path){
    QString result;

    int iRet = 0;
    std::string msg = "";
    char* temppath;
    QByteArray ba = path.toLatin1();
    temppath = ba.data();
    iRet = GetDirOrFile(temppath/*"//CNC_MEM/USER/"*/, msg, 2);
    if (iRet < 0)
    {
        return result;
    }
    if ( msg.rfind("\x02") > 0)
    {
        msg.erase(msg.rfind("\x02"));
    }
    result = QString::fromStdString(msg);

    return result;
}

bool FanucCNC::SetNcProgramByPath(QString code,QString path){
    bool isSuccess = false;

    if ((code == NULL) || (code == ""))
    {
        return false;
    }
    if ((path == NULL) || (path == ""))
    {
        return false;
    }

    if(!path.contains("//CNC_MEM/"))
    {
        path = OldPath + path;
    }

    char* temppath;
    QByteArray ba1 = path.toLatin1();
    temppath = ba1.data();

    char* tempcode;
    std::string strcode = std::string((const char*)code.toLatin1(), code.size());
    tempcode = (char *)strcode.c_str();

    // 下载前检测磁盘剩余空间
    ODBNC buf;
    short ret = cnc_rdproginfo(h, 0, 16, &buf);
    if ((ret == EW_OK) && (buf.u.bin.unused_mem <= 0))
    {
        return false;
    }

    std::string pathStr = "";
    pathStr.append(temppath);

    int pos = pathStr.rfind('/');
    std::string dir = pathStr.substr(0, pos);
    std::string filename = pathStr.substr(pos+1, pathStr.length() - pos - 1);

    int codeLen = strcode.size();
    char param3202 = 0;
    bool wrProtect = false;
    if ((filename[1] == '8') || (filename[1] == '9'))
    {
        char iodbpsdBuf[12];
        IODBPSD* iodbpsd = (IODBPSD*)iodbpsdBuf;

        memset(iodbpsd, 0, 5);
        short ret = cnc_rdparam(h, 3202, 0, 5, iodbpsd);
        if (ret != EW_OK)
        {
            ;
        }
        else
        {
            param3202 = iodbpsd->u.cdata;
            if ((param3202 & 17) == 17)
            {
                wrProtect = true;
                iodbpsd->datano = 3202;
                iodbpsd->type = 0;
                iodbpsd->u.cdata = (param3202 & (~17));
                if (EW_OK != cnc_wrparam(h, 5, iodbpsd))
                {
                    ;
                }
            }
        }
    }

    bool mp = false;
    char buf512[512] = { 0 };
    ret = cnc_pdf_rdmain(h, buf512);
    if (ret == EW_OK)
    {
        if (strcmp(temppath, buf512) == 0)
        {
            mp = true;
            std::string mainPrg = GetRandomMainProgram();
            ret = cnc_pdf_slctmain(h, (char*)mainPrg.c_str());
            if (ret != EW_OK)
            {
                ODBERR C;
                ret = cnc_getdtailerr(h, &C);
                if (ret == EW_OK)
                {
                    return false;
                }
            }
        }
    }

    //del
    for (int i = 0; i < 5; i++)
    {
        short ret = cnc_pdf_del(h, temppath);
        //if (ret != EW_OK)
        //{
        //	if (i >= 9)
        //	{
        //		char buf[256] = { 0 };
        //		sprintf(buf, "Failed to del downloading nc program. %s", path);
        //		HandleErrorEx(ret, "Error", buf, true, 1, msgHandler, userToken);
        //		break;
        //	}
        //}
        if (ret == EW_OK)
            break;

#ifdef Q_OS_WIN
            Sleep(10);
#else
            struct timespec ts = {10 / 1000, (10 % 1000) * 1000 * 1000};
            nanosleep(&ts, NULL);
#endif
    }

    //start
    for (int i = 0; i < 10; i++)
    {
        short ret = cnc_dwnstart4(h, 0, (char*)dir.c_str());
        if (ret != EW_OK)
        {
            if (i >= 9)
            {
                ret = cnc_dwnend4(h);
                return false;
            }
        }
        if (ret == EW_OK)
            break;

#ifdef Q_OS_WIN
        Sleep(100);
#else
        struct timespec ts = {100 / 1000, (100 % 1000) * 1000 * 1000};
        nanosleep(&ts, NULL);
#endif
    }

    //MsgOut(msgHandler, userToken, "Path", filePath, true, 0);//only for standalone test
    int retryCount = 0;
    int count = 0;
    long len = 0;
    ODBERR C;
    while (true)
    {
        len = codeLen - count;
        if (len > 1280)
        {
            len = 1280;
        }
        ret = cnc_download4(h, &len, &tempcode[count]);
        if (ret == EW_BUFFER)
        {
#ifdef Q_OS_WIN
            Sleep(50);
#else
            struct timespec ts = {50 / 1000, (50 % 1000) * 1000 * 1000};
            nanosleep(&ts, NULL);
#endif
            retryCount++;
            if (retryCount < 1000)
            {
                continue;
            }
        }

        if (ret == EW_OK)
        {
            retryCount = 0;
            count += len;
        }
        else
        {
            cnc_getdtailerr(h, &C);

            if (ret == EW_RESET)
            {
                isSuccess = false;
                break;
            }
            else
            {
                if (C.err_dtno == 10053)
                {
                    isSuccess = false;
                    break;
                }

                retryCount++;
                if (retryCount >= 10)
                {
                    isSuccess = false;
                    break;
                }
            }
        }

        if (codeLen <= count)
        {
            isSuccess = true;
            break;
        }
    }

    //end
    ret = cnc_dwnend4(h);
    if (ret != EW_OK)
    {
        return false;
    }

    if (!isSuccess)
    {
        cnc_resetconnect(h);
#ifdef Q_OS_WIN
        Sleep(100);
#else
        struct timespec ts = {100 / 1000, (100 % 1000) * 1000 * 1000};
        nanosleep(&ts, NULL);
#endif
        return false;
    }

    if (wrProtect)//recover value of parameter 3202
    {
        char iodbpsdBuf[12];
        IODBPSD* iodbpsd = (IODBPSD*)iodbpsdBuf;
        iodbpsd->datano = 3202;
        iodbpsd->type = 0;
        iodbpsd->u.cdata = param3202;
        if (EW_OK != cnc_wrparam(h, 5, iodbpsd))
        {
            ;
        }
    }

    if (mp)
    {
        ret = cnc_pdf_slctmain(h, temppath);
        if (ret != EW_OK)
        {
            ODBERR C;
            ret = cnc_getdtailerr(h, &C);
            if (ret == EW_OK)
            {
                ;
            }
        }
    }
    return  isSuccess;
}

bool FanucCNC::DelNcProgramByPath(QString path)
{
    if(!path.contains("//CNC_MEM/"))
    {
        path = OldPath + path;
    }
    char* temppath;
    QByteArray ba1 = path.toLatin1();
    temppath = ba1.data();
    short ret = cnc_pdf_del(h, temppath);
    if (ret != EW_OK)
    {
        ODBERR C;
        ret = cnc_getdtailerr(h, &C);
        return false;
    }
    else
    {
        return true;
    }
}

