/*
* CMPS104A
* Created by:
*  Tyler Hoang (tydhoang@ucsc.edu)
*  Eric Vin (evin@ucsc.edu)
* string_set.h
*/
#ifndef __STRING_SET__
#define __STRING_SET__

#include <string>
#include <unordered_set>

#include <stdio.h>

struct string_set {
   string_set();
   static std::unordered_set<std::string> set;
   static const std::string* intern (const char*);
   static void dump (FILE*);
};

#endif

