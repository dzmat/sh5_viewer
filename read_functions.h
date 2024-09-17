#ifndef READ_FUNCTIONS_H
#define READ_FUNCTIONS_H
#include <QString>
#include "coords.h"
#include "support_classes.h"

const int MY_EOF = -1;

inline int read_char(char &ch);
int my_read_line(std::string &str);
bool init_read(const QString &fname);
void close_read();
void unread_entity_list1(TStringList &list);
bool check_header(std::string &instr, const std::string &type);
bool check_for_my_unit(const TStringList &ls);
/**
 * @brief read_entity_list1
 * @param list (out) TStringList receives all strings of block to parse
 * @return number of non-head lines. -1 if EOF
 */
int read_entity_list1(TStringList &list);
bool check_for_group(const TStringList &ls);
double read_double(const TStringList &ls, const std::string &key);
void coord_load(const TStringList &ls, TGameCoord &c);
#endif // READ_FUNCTIONS_H
