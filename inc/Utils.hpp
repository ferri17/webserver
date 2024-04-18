
#pragma once

#include <vector>
#include <iostream>
#include <sstream>

#define KO -1
#define NOT_INIT 0
#define MISS_KEY 1
#define OPEN_KEY 2
#define COMENT 3
#define EMPTY 4
#define CLOSE_KEY 5
#define MISS_KEY_LOC 6
#define OPEN_KEY_LOC 7
#define CLOSE_KEY_LOC 8
#define VALID_ARG 10
#define VALID_ARG_LOC 11

#define BYTES 0
#define KILOS 1
#define MEGAS 2
#define GIGAS 3

#define CR '\r'
#define LF '\n'
#define SP ' '
#define HTAB '\t'
#define COLON ':'
#define HYPHEN '-'
#define USCORE '_'

// Removes duplicated delimiters
std::vector<std::string> split(const std::string& s, char delimiter);
// Stores as an empty string repeated delimiters
std::vector<std::string> split_r(const std::string& s, char delimiter);
bool	isInt(const std::string & value);
bool	isUsAscii(int c);
bool	isOWS(int c);
std::string	stringToLower(std::string str);
std::string toString(int num);