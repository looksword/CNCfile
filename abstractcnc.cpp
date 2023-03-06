#include "abstractcnc.h"

AbstractCNC::AbstractCNC(QString ip,QString user, QString pass)
{
    this->ip = ip;
    this->user = user;
    this->pass = pass;
}
