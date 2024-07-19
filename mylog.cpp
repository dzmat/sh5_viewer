#include "mylog.h"

namespace mylogger
{
    using std::string;

    QTextEdit *logptr;

    void log(const QString &msg)
    {
        logptr->append(msg);
    }

    void logs(const string &msg)
    {
        QString s(msg.c_str());
        log(s);
    }
} // namespace mylogger
