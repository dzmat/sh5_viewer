#include "tworld.h"

#include <fileapi.h>
#include <wtypes.h>
#include <winbase.h>
#include <windows.h>


typedef void *HANDLE;
HANDLE file_handle;
const int MY_EOF = -1;
const int MY_BUF_SZ = 1024;
char buf[MY_BUF_SZ];
unsigned long buf_count, buf_pos;

TWorld::TWorld()
{
}

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


void init_read()
{
    buf_pos = buf_count = 0;// init for read_char
    unread_buf.clear();
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

void coord_load(TGameCoord &c, const TStringList &ls)
{
    c.x = read_double(ls, "Long");
    c.y = read_double(ls, "Lat");
}

double line_min_distance(const TGameCoord &a, const TGameCoord &b, const TGameCoord &d)
{
    if ( (a - b).len_sqr() < 1e-6) return (d - a).len(); // too short line, dont calculate projection.
    TGameCoord v = (b - a);
    double proj = (v * (d - a)) / v.len_sqr();
    if (proj < 0) return (d - a).len();
    if (proj > 1) return (d - b).len();
    return (a + v * proj - d).len();// nearest point is somewhere in the line cut
}

double TWay::min_distance_to(const TGameCoord &dest) const
{
    size_t size = data.size();
    double min_r = 1e99;
    for (size_t i = 0; i < size - 1; i++) {
        double r = line_min_distance(data[i].coord, data[i + 1].coord, dest);
        if (r < min_r) min_r = r;
    }
    return min_r;
}

bool TGroup::has_type(int type)
{
    if (type <= 0) return true;
    for (auto &u : units)
        if (u.type == type) return true;
    return false;
}

bool TGroup::has_zero_speed()
{
    for (auto &u : units)
        if (u.speed < 0.1) return true;
    return false;
}

void TGroup::load_units(std::string command_unit_name)
{

    TStringList ls;
    for (size_t i = 0; i < size(); ) {
        read_entity_list1(ls);
        std::string head = ls[0];
        unread_entity_list1(ls);
        if (head.find("Waypoint") != std::string::npos) {
            way.load();
        }
        else {
            units[i].load(command_unit_name);
            i++;
            // try to load folloving waypoints
            read_entity_list1(ls);
            std::string head = ls[0];
            unread_entity_list1(ls);
            if (head.find("Waypoint") != std::string::npos)
                way.load();
        }
    }
    // extract next waypoint info
    for (TUnit &unit : units) {
        if (unit.is_command)
            way.next_way_point = unit.next_waypoint;
        break;
    }
}

void TWay::load()
{
    TStringList ls;
    int waypoint_index = 1;
    while (1) {
        int res = read_entity_list1(ls);
        if (res == MY_EOF) return;
        if (ls[0].find("Waypoint ") != std::string::npos) {
            TGameCoord t;
            coord_load(t, ls);
            data.push_back(TWayPoint{waypoint_index++, t});
        }
        else {
            unread_entity_list1(ls);
            break;// end of waypoints list
        }
    }
}

void TUnit::load(std::string expected_command_unit_name)
{
    TStringList ls;
    int res = read_entity_list1(ls);
    if (res == MY_EOF) return;
    load(ls, expected_command_unit_name);
}

void TUnit::load(const TStringList &ls, const std::string expected_command_unit_name)
{
    //    try{
    heading = read_double(ls, "Heading");

    speed = read_double(ls, "Speed");
    s_name = ls.getValue("Name");
    s_class = ls.getValue("Class");
    type = stoi(ls.getValue("Type"));
    next_waypoint = stoi(ls.getValue("NextWP", "0"));
    if (s_name == expected_command_unit_name)
        is_command = true;
    coord_load(coord, ls);
    if ((100 <= type) && (type <= 199)) is_warship = false; else is_warship = true;
    if (ls.getValue("Origin") == "German") is_german = true; else is_german = false;
    //    }catch(EConvertError& E){
    //        QMessageBox::warning(NULL,"Exception encountered","exception while loading Unit");
    //    };
}

void TUnit::dump(QTextEdit *d) const
{
    d->append(QString("Type=%1 class=%2 name=%3").arg(type).arg(s_class.c_str(), s_name.c_str()));
    d->append(QString("Heading: %1 Speed: %2").arg(heading, 0, 'f').arg(speed, 0, 'f'));
    d->append(QString("Long: %1 Lat: %2").arg(coord.x, 0, 'f').arg(coord.y, 0, 'f'));
}

int TWorld::load_entity()
{
    TStringList ls;
    int res = read_entity_list1(ls);
    if (res == MY_EOF) return MY_EOF;
    if (check_for_group(ls)) {
        load_group(ls);
        return 1;
    }
    if (check_for_my_unit(ls)) {
        load_my_unit(ls);
        my_units++;
        return 1;
    }
    return 0;
}

bool TWorld::load_file(const QString &fname)
{
    std::wstring ws_fname = fname.toStdWString();
    file_handle = CreateFile(
        ws_fname.c_str(),
        GENERIC_READ, 0, NULL, OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (file_handle == INVALID_HANDLE_VALUE)
        return false;
    buf_pos = buf_count = 0;// init for read_char
    init_read();
    reset_groups();
    my_units = 0;
    int res, cnt = 0;
    while (1) {
        res = load_entity();
        if (res == MY_EOF) break;
        cnt += res;;
    }
    if (cnt == 0)
        return false;
    mylogger::log(QString("loaded entities: %1").arg(cnt));
    mylogger::log(QString("loaded groups: %1").arg(groups.size()));
    mylogger::log(QString("loaded units: %1").arg(my_units));
    CloseHandle(file_handle);
    return true;
}
