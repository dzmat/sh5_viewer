#ifndef MYLOG_H
#define MYLOG_H

#include <QTextEdit>

namespace mylogger
{
    extern QTextEdit *logptr;
    void log(const QString &msg);
    void logs(const std::string &msg);
};

#endif // MYLOG_H
