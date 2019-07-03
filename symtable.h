/*
* CMPS104A
* Created by:
*  Tyler Hoang (tydhoang@ucsc.edu)
*  Eric Vin (evin@ucsc.edu)
* symtable.h
*/

#ifndef symtable_h
#define symtable_h

#include <vector>
#include <stdio.h>
#include <vector>
#include <bitset>
#include <unordered_map>
#include <iostream>
#include <string.h>

#include "auxlib.h"
#include "lyutils.h"
#include "astree.h"

using namespace std;
struct symbol;

enum {
    VOID, INT, NULLPTR_T, STRING, STRUCT, ARRAY, FUNCTION, VARIABLE,
    FIELD, TYPEID, PARAM, LOCAL, LVAL, CONST, VREG, VADDR, PTR, BITSET_SIZE,
};

using attr_bitset = bitset<BITSET_SIZE>;
using symbol_table = unordered_map<string,symbol*>;

struct symbol {
    attr_bitset attributes;
    size_t sequence;
    symbol_table fields;
    location lloc;
    size_t block_nr = 0;
    vector <symbol*> parameters;
    void dump(FILE* outfile);
    astree* origin;
    string struct_name;
    string string_name;
    string param_name;
    string param_type;
    string local_type;
};

using symbol_entry = symbol_table::value_type;

void set_attributes(astree* node);
symbol* new_symbol(astree* node);
symbol* new_function(astree* node);
void make_symbol_table (astree* tree);
void crawl_tree(FILE* outfile, astree* tree);
void make_symbol_table(FILE* outfile, astree* tree);
void dump(FILE* outfile);

#endif /* symtable_h */