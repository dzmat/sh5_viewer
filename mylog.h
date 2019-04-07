#ifndef MYLOG_H
#define MYLOG_H

#include <QTextEdit>

namespace mylogger
{
    extern QTextEdit *logptr;
    void log(QString msg);
    void logs(std::string msg);
};

#endif // MYLOG_H
