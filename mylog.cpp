#include "mylog.h"

namespace mylogger
{
    QTextEdit *logptr;

    void log(QString msg)
    {
        logptr->append(msg);
    }

    void logs(std::string msg)
    {
        QString s(msg.c_str());
        log(s);
    }
};
