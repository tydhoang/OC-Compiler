/*
* CMPS104A
* Created by:
*  Tyler Hoang (tydhoang@ucsc.edu)
*  Eric Vin (evin@ucsc.edu)
* oilconvert.h
*/
#ifndef oilconvert_h
#define oilconvert_h

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
#include "symtable.h"

void outputOil(FILE* outfile);
void functionCrawl(FILE* outfile, astree* startBlock);

#endif