#include "read_functions.h"
#include "mylog.h"
#include <windows.h>

typedef void *HANDLE;
HANDLE file_handle;

const int MY_BUF_SZ = 1024;
char buf[MY_BUF_SZ];
unsigned long buf_count, buf_pos;


inline int read_char(char &ch)
{
    if ((buf_pos >= buf_count) || (buf_count == 0)) {
        // pump from disk
        bool res = ReadFile(file_handle, buf, MY_BUF_SZ, &buf_count, 0);
        if (!res || (res && (buf_count == 0))) {
            if (!res) {
                DWORD err = GetLastError();
                wchar_t msg[2048];
                FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                              NULL, err, 0, msg, 2048, NULL);
                mylogger::log(QString::fromWCharArray(msg, 2048));
            }
            ch = 0;
            return 0;// EOF
        }
        buf_pos = 0;
    }
    ch = buf[buf_pos];
    buf_pos++;
    return 1;
}

int my_read_line(std::string &str)
{
    char buf[MY_BUF_SZ + 4]; // +4 for preventing accidental write
    // beyond array border while skipping '\n'
    int i = 0;
    bool eof_flag = false;
    while (i < MY_BUF_SZ) {
        if (!read_char(buf[i]) ) {
            eof_flag = true;
            break;
        }
        if (buf[i] == '\r') {
            read_char(buf[i + 1]);// skip '\n'
            break;
        }
        i++;
    }
    if ((i == 0) && eof_flag) // haven't read anything, already at eof
        return MY_EOF;
    str = std::string(buf, i);
    return str.length();
}

TStringList unread_buf;


bool init_read(const QString &fname)
{

    std::wstring ws_fname = fname.toStdWString();
    file_handle = CreateFile(
        ws_fname.c_str(),
        GENERIC_READ, 0, NULL, OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (file_handle == INVALID_HANDLE_VALUE)
        return false;
    buf_pos = buf_count = 0;// init for read_char


    buf_pos = buf_count = 0;// init for read_char
    unread_buf.clear();
    return true;
}

void close_read()
{
    CloseHandle(file_handle);
}

void unread_entity_list1(TStringList &list)
{
    unread_buf = list;
}

bool check_header(std::string &instr, const std::string &type)
{
    int ilen = instr.length();
    int tlen = type.length();
    if (ilen < tlen + 2) return false;
    if ((instr[0] != '[') || (instr[ilen - 1] != ']'))
        return false;
    std::string t = instr.substr(1, ilen - 2);
    if (t.substr(0, tlen) != type)
        return false;
    std::string tt = t.substr(tlen, t.length() - tlen);
    for (auto &c : tt)
        if (!isdigit(c))
            return false;
    return true;
}

bool check_for_my_unit(const TStringList &ls)
{
    std::string s = ls[0];
    if (!check_header(s, "Unit ")) return false;
    std::string t = ls.getValue("Commander");
    if (t != std::string("1")) return false;
    return true;
}

/**
 * @brief read_entity_list1
 * @param list (out) TStringList receives all strings of block to parse
 * @return number of non-head lines. -1 if EOF
 */
int read_entity_list1(TStringList &list)
{
    if (unread_buf.size()) {
        list = unread_buf;
        unread_buf.clear();
        return 1;
    }
    std::string s;
    bool flag = false;
    list.clear();
    // seek section begin
    while (my_read_line(s) != MY_EOF) {
        if (s.c_str()[0] == '[') {// found section beginning
            flag = true;
            list.push_back(s);
            break;
        }
    }
    if (!flag) return MY_EOF; // no section beginning found
    // read body
    int cnt = 0;
    while (my_read_line(s) != MY_EOF) {
        if (s.length() == 0) // end of body - empty string
            break;
        list.push_back(s);
        cnt++;
    }
    return cnt;
}

bool check_for_group(const TStringList &ls)
{
    std::string s = ls[0];
    return check_header(s, "Group ");
}

double read_double(const TStringList &ls, const std::string &key)
{
    std::string t = ls.getValue(key);
    if (t.length() == 0) return 0.0;
    return stod(t);
}

void coord_load(const TStringList &ls, TGameCoord &c)
{
    c.x = read_double(ls, "Long");
    c.y = read_double(ls, "Lat");
}
