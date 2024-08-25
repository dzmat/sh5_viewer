#include "mylog.h"

namespace mylogger
{
    using std::string;

    QTextEdit *logptr;

/**
 * Appends a message to the log text edit widget.
 *
 * @param msg The message to be appended to the log.
 *
 * @return void
 *
 * @throws None
 */
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


