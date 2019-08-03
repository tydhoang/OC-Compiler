/*
* Created by:
*  Tyler Hoang (tydhoang@ucsc.edu)
*  Eric Vin (evin@ucsc.edu)
* oilconvert.cpp
*/
#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lyutils.h"
#include "oilconvert.h"
#include "symtable.h"

extern symbol_table globalSymTable;
extern symbol_table localSymTable;
int whcounter = 0;
int ifcounter = 0;
int thcounter = 0;
int docounter = 0;
int elcounter = 0;
int ficounter = 0;
int odcounter = 0;

void outputOil (FILE* outfile) {
    int stringCounter = 0;

    for ( auto i = globalSymTable.begin(); i != globalSymTable.end(); i++ ) {
        if(i->second->attributes[STRING] == 1 && i->second->attributes[LOCAL] == 1) {
            fprintf(outfile, ".s%d:     %s\n", stringCounter, i->second->string_name.c_str());
            stringCounter++;
        }

        else if (i->second->attributes[STRING] == 1) {
            fprintf(outfile, ".s%d:     %s\n", stringCounter, i->second->origin->children[2]->lexinfo->c_str());
            stringCounter++;
        }

        else if (i->second->origin->children.size() == 3 && i->second->attributes[PTR] == 0 && i->second->attributes[INT] == 1) {
            fprintf(outfile, "%-5s:     .global int %s\n", i->first.c_str(), i->second->origin->children[2]->lexinfo->c_str());
        }
        else if (i->second->origin->children.size() == 2 && i->second->attributes[PTR] == 0 && i->second->attributes[INT] == 1) {
            fprintf(outfile, "%-5s:     .global int\n", i->first.c_str());
        }
        else if (i->second->attributes[PTR] == 1) {
            if (i->second->origin->children.size() == 3) {
                fprintf(outfile, "%-5s:     .global ptr %s\n", i->first.c_str(), i->second->origin->children[2]->lexinfo->c_str());
            }
            if (i->second->origin->children.size() == 2) {
                fprintf(outfile, "%-5s:     .global ptr\n", i->first.c_str());
            }
        }

        else if (i->second->attributes[FUNCTION] == 1) {
            fprintf(outfile, "%-5s:  .function %s\n", i->first.c_str(), i->second->origin->children[0]->children[0]->lexinfo->c_str());
            for(unsigned int x = 0; x < i->second->parameters.size(); x++) {
                fprintf(outfile, "          .param %s %s\n", i->second->parameters[x]->param_type.c_str(), i->second->parameters[x]->param_name.c_str());
            }
            for ( auto j = localSymTable.begin(); j != localSymTable.end(); j++ ) {
                if (j->second->attributes[LOCAL] == 1) {
                    if (j->second->block_nr == i->second->block_nr) {
                        fprintf(outfile, "          .local %s %s\n", j->second->local_type.c_str(), j->first.c_str());
                    }
                }
            }

            //Begin function crawl
            if (i->second->origin->children.size() == 2) {
                continue;
            }
            astree* funcBlock = i->second->origin->children[2];
            functionCrawl(outfile, funcBlock);
            fprintf(outfile, "          .end");
        }
        fprintf(outfile, "\n");
    }
}

void functionCrawl(FILE* outfile, astree* startBlock) {
    switch(startBlock->symbol) {
        case '=' : {

            if (startBlock->children.size() == 2) {
                fprintf(outfile, "          %s = %s()\n", startBlock->children[0]->lexinfo->c_str(), startBlock->children[1]->lexinfo->c_str());
                break;
            }
            fprintf(outfile, "          %s = %s", startBlock->children[0]->lexinfo->c_str(), startBlock->children[1]->children[0]->lexinfo->c_str());

            switch(startBlock->children[1]->symbol) {
                case '+' : {
                    fprintf(outfile, " + ");
                    break;
                }
                case '-' : {
                    fprintf(outfile, " - ");
                    break;
                }
                case '*' : {
                    fprintf(outfile, " * ");
                    break;
                }
                case '/' : {
                    fprintf(outfile, " / ");
                    break;
                }
            }

            fprintf(outfile, "%s\n", startBlock->children[1]->children[1]->lexinfo->c_str());

            break;
        }

        case (TOK_WHILE) : {
            fprintf(outfile, ".wh%d:     goto .od%d if ", whcounter, odcounter);
            if (strcmp(startBlock->children[0]->lexinfo->c_str(), "!=") == 0) {
                fprintf(outfile, "%s == %s\n", startBlock->children[0]->children[0]->lexinfo->c_str(),startBlock->children[0]->children[1]->lexinfo->c_str());
            }
            else if (strcmp(startBlock->children[0]->lexinfo->c_str(), "==") == 0) {
                fprintf(outfile, "%s != %s\n", startBlock->children[0]->children[0]->lexinfo->c_str(),startBlock->children[0]->children[1]->lexinfo->c_str());
            }
            else if (strcmp(startBlock->children[0]->lexinfo->c_str(), ">") == 0) {
                fprintf(outfile, "%s <= %s\n", startBlock->children[0]->children[0]->lexinfo->c_str(),startBlock->children[0]->children[1]->lexinfo->c_str());
            }
            else if (strcmp(startBlock->children[0]->lexinfo->c_str(), "<") == 0) {
                fprintf(outfile, "%s >= %s\n", startBlock->children[0]->children[0]->lexinfo->c_str(),startBlock->children[0]->children[1]->lexinfo->c_str());
            }
            else if (strcmp(startBlock->children[0]->lexinfo->c_str(), "<=") == 0) {
                fprintf(outfile, "%s > %s\n", startBlock->children[0]->children[0]->lexinfo->c_str(),startBlock->children[0]->children[1]->lexinfo->c_str());
            }
            else if (strcmp(startBlock->children[0]->lexinfo->c_str(), ">=") == 0) {
                fprintf(outfile, "%s < %s\n", startBlock->children[0]->children[0]->lexinfo->c_str(),startBlock->children[0]->children[1]->lexinfo->c_str());
            }
            else if (strcmp(startBlock->children[0]->lexinfo->c_str(), "1") == 0) {
                fprintf(outfile, "FALSE\n");
            }
            else if (strcmp(startBlock->children[0]->lexinfo->c_str(), "0") == 0) {
                fprintf(outfile, "TRUE\n");
            }

            fprintf(outfile, ".do%d:\n", docounter);
            functionCrawl(outfile, startBlock->children[1]);
            whcounter++;
            docounter++;
            ficounter++;
            fprintf(outfile, ".fi%d:\n", ficounter);
            fprintf(outfile, ".od%d:\n", odcounter);
            odcounter++;
            break;
        }

        case (TOK_IF) : {
            ifcounter++;
            thcounter++;
            elcounter++;
            fprintf(outfile, ".if%d:     ", ifcounter);
             if (strcmp(startBlock->children[0]->lexinfo->c_str(), "!=") == 0) {
                 if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "%") == 0) {
                     printf ("%s %% %s != %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "+") == 0) {
                     printf ("%s + %s != %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "-") == 0) {
                     printf ("%s - %s != %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "*") == 0) {
                     printf ("%s * %s != %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "/") == 0) {
                     printf ("%s / %s != %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else {
                    fprintf(outfile, "%s != %s\n", startBlock->children[0]->children[0]->lexinfo->c_str(),startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
            }
            else if (strcmp(startBlock->children[0]->lexinfo->c_str(), "==") == 0) {
                if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "%") == 0) {
                     printf ("%s %% %s == %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "+") == 0) {
                     printf ("%s + %s == %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "-") == 0) {
                     printf ("%s - %s == %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "*") == 0) {
                     printf ("%s * %s == %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "/") == 0) {
                     printf ("%s / %s == %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else {
                    fprintf(outfile, "%s == %s\n", startBlock->children[0]->children[0]->lexinfo->c_str(),startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
            }
            else if (strcmp(startBlock->children[0]->lexinfo->c_str(), ">") == 0) {
                if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "%") == 0) {
                     printf ("%s %% %s > %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "+") == 0) {
                     printf ("%s + %s > %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "-") == 0) {
                     printf ("%s - %s > %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "*") == 0) {
                     printf ("%s * %s > %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "/") == 0) {
                     printf ("%s / %s > %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else {
                    fprintf(outfile, "%s > %s\n", startBlock->children[0]->children[0]->lexinfo->c_str(),startBlock->children[0]->children[1]->lexinfo->c_str());
                 };
            }
            else if (strcmp(startBlock->children[0]->lexinfo->c_str(), "<") == 0) {
                if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "%") == 0) {
                     printf ("%s %% %s < %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "+") == 0) {
                     printf ("%s + %s < %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "-") == 0) {
                     printf ("%s - %s < %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "*") == 0) {
                     printf ("%s * %s < %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "/") == 0) {
                     printf ("%s / %s < %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else {
                    fprintf(outfile, "%s < %s\n", startBlock->children[0]->children[0]->lexinfo->c_str(),startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
            }
            else if (strcmp(startBlock->children[0]->lexinfo->c_str(), "<=") == 0) {
                if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "%") == 0) {
                     printf ("%s %% %s <= %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "+") == 0) {
                     printf ("%s + %s <= %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "-") == 0) {
                     printf ("%s - %s <= %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "*") == 0) {
                     printf ("%s * %s <= %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "/") == 0) {
                     printf ("%s / %s <= %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else {
                    fprintf(outfile, "%s <= %s\n", startBlock->children[0]->children[0]->lexinfo->c_str(),startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
            }
            else if (strcmp(startBlock->children[0]->lexinfo->c_str(), ">=") == 0) {
                if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "%") == 0) {
                     printf ("%s %% %s >= %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "+") == 0) {
                     printf ("%s + %s >= %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "-") == 0) {
                     printf ("%s - %s >= %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "*") == 0) {
                     printf ("%s * %s >= %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else if (strcmp(startBlock->children[0]->children[0]->lexinfo->c_str(), "/") == 0) {
                     printf ("%s / %s >= %s\n", startBlock->children[0]->children[0]->children[0]->lexinfo->c_str(), startBlock->children[0]->children[0]->children[1]->lexinfo->c_str(), startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
                 else {
                    fprintf(outfile, "%s >= %s\n", startBlock->children[0]->children[0]->lexinfo->c_str(),startBlock->children[0]->children[1]->lexinfo->c_str());
                 }
            }
            fprintf(outfile, "          goto .el%d if not\n", elcounter);
            fprintf(outfile, ".th%d:", thcounter);
            functionCrawl(outfile, startBlock->children[1]);
            break;
        }
        case (TOK_RETURN) : {
            fprintf(outfile, "          return %s\n", startBlock->children[0]->lexinfo->c_str());
            fprintf(outfile, "          return\n");
            break;
        }
        default : {
            for(unsigned int i = 0; i < startBlock->children.size(); i++) {
                functionCrawl(outfile, startBlock->children[i]);
            }
        }
    }
}       