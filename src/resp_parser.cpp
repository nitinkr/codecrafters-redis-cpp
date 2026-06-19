#include <cctype>
#include <cstdint>
#include <iostream>

#include "resp_parser.h"

void to_upper(std::string &str) {
    for(int i=0; i<str.length(); i++) {
        str[i] = std::toupper(str[i]);
    }
}

bool RespParser::parse() {
    if(is_end()) return is_valid_;
    char ch = peek();
    std::cout << "parse " << ch  << std::endl;
    switch(ch) {
        case '*':
            parse_array();
            break;
        case '+':
            parse_simple_string();
            break;
        case '$':
            parse_bulk_string();
            break;
        case ':':
            parse_int64();
        default:
            is_valid_ = false;
    }
    return is_valid_;
}

void RespParser::parse_array() {
    std::cout << "parse_array before match " << buff_[current_] << std::endl;
    if(!match('*')) return;
    std::cout << "parse_array " << std::endl;
    int len = parse_length();
    std::cout << "parse_array len " << len << std::endl;
    if (len < 0) {
        is_valid_ = false;
        return;
    }

    while(len--) { parse(); }
}

void RespParser::parse_int64() {
   if(!match(':')) return;
   int64_t value = 0;
   bool neg = match('-');
   match('+');
   char ch;
   while(!is_crlf()) {
      ch = advance();
      value  = value * 10 + (ch - '0');
   }
   RespValue v;
   v.value_ = value;
   v.type_  = RespValue::INTEGER64;
   values_.push_back(v);
}

int RespParser::parse_length() {
    int len = 0;
    bool neg = false;
    char ch;
    while(!is_end() && !is_crlf()) {
       ch = advance();
       if(ch == '-') {
           neg = true;
           continue;
       }
       if(!std::isdigit(ch)) {
           return -100;
       }
       len = len * 10 + (ch - '0');
    }
    return (!neg) ? len : -1*len;
}

// parse simple bulk strings: $<len>\r\n<str_value>\r\n
void RespParser::parse_bulk_string() {
    std::cout << "in parse_bulk_string " << std::endl;
    if(!match('$')) return;
    int len = parse_length();
    std::cout << "in parse_bulk_string after match len " << len << std::endl;
    RespValue v;
    std::string value;
    v.type_ = RespValue::STRING;
    if(current_ + len >= length_ || len < -1) {
        std::cout << "current_ " << current_ << " len " << len << " length_ " << length_ << std::endl;
        is_valid_ = false;
        return;
    }
    std::cout << "have enough space " << std::endl;
    if(len == -1) {
        v.type_ = RespValue::NULL_STRING;
    }
    for(int i=0; i<len; i++) {
        value.push_back(advance());
    }
    is_valid_ = is_crlf();
    std::cout << "value " << value << std::endl;
    v.value_ = std::move(value);
    values_.push_back(v);
}

// parse simple resp strings: +<str_value>\r\n
void RespParser::parse_simple_string() {
    std::cout << "in parse_simple_string " << buff_[current_] << std::endl;
    if(!match('+')) return;
    std::cout << "matches + " << std::endl;
    RespValue v;
    v.type_ = RespValue::STRING;
    bool found = false;
    std::string value;
    while(!is_end()) {
       if(is_crlf()) {
           found = true;
           break;
       }
       value.push_back(advance());
    }
    std::cout << "value " << value << " found " << found << std::endl;
    if(!found) {
        is_valid_ = false;
        return;
    }
    std::cout << "found string " << value << std::endl;
    v.value_ = std::move(value);
    values_.push_back(v);
}

char RespParser::peek() {
    return (is_end()) ? '\0' : buff_[current_];
}

bool RespParser::match(char ch) {
    if(peek() != ch) return false;
    advance();
    return true;
}

char RespParser::advance() {
    return buff_[current_++];
}
bool RespParser::is_crlf() { 
    if(!is_end() && current_ + 1 < length_ && buff_[current_] == '\r' && buff_[current_ + 1] == '\n') {
        advance();
        advance();
        return true;
    }
    return false;
}
