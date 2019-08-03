/*
* Created by:
*  Tyler Hoang (tydhoang@ucsc.edu)
*  Eric Vin (evin@ucsc.edu)
* symtable.cpp
*/
#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lyutils.h"
#include "symtable.h"
#include "oilconvert.h"

symbol_table globalSymTable;
symbol_table localSymTable;

//Declares and initializes block counter
int blockNum = 1;

void make_symbol_table(FILE* outfile, astree* tree) {
    crawl_tree(outfile, tree);
}

void symbol::dump(FILE* outfile) {
  
    vector<string> attributeNames = {"VOID", "INT", "NULLPTR_T", "STRING", "STRUCT", "ARRAY", "FUNCTION", "VARIABLE",
    "FIELD", "TYPEID", "PARAM", "LOCAL", "LVAL", "CONST", "VREG", "VADDR"};
    for (int i = 0; i < 16; i++) {
        if(attributes[i] == 1) {
            fprintf(outfile, "%s ", attributeNames[i].c_str());
        }
    }
}

void crawl_tree(FILE* outfile, astree* tree) {
    //Gets token name from parser
    const char* tokenName = parser::get_tname (tree->symbol);

    //Declares and initializes global symbol table

    //Checks token name against relevant tokens
    //Declaration tokens
    if(strcmp(tokenName, "TOK_TYPE_ID") == 0) {
        astree* targetToken = tree->children[1];
        string targetTokenName = targetToken->lexinfo->c_str();
        astree* targetTypeToken = tree->children[0];

        //Checks if declaration already exists in symbol table
        if(globalSymTable.find(targetTokenName) != globalSymTable.end()) {
            fprintf(outfile, "Error\n");
            exit(EXIT_FAILURE);
        }

        symbol* targetSymbol = new_symbol(targetToken);
        targetSymbol->origin = tree;
        targetSymbol->attributes[VARIABLE] = 1;

        switch (targetTypeToken->symbol) {
            case TOK_VOID : {
                targetSymbol->attributes[VOID] = 1;
                break;
            }
            case TOK_INT : {
                targetSymbol->attributes[INT] = 1;
                targetSymbol->attributes[LVAL] = 1;
                break;
            }
            case TOK_STRING : {
                targetSymbol->attributes[STRING] = 1;
                targetSymbol->attributes[LVAL] = 1;
                break;
            }
            case TOK_NULLPTR : {
                targetSymbol->attributes[NULLPTR_T] = 1;
                targetSymbol->attributes[CONST] = 1;
                break;
            }
            case TOK_ARRAY : {
                targetSymbol->attributes[ARRAY] = 1;
                break;
            }
            case TOK_PTR : {
                targetSymbol->attributes[PTR] = 1;
                targetSymbol->struct_name = tree->children[0]->children[0]->lexinfo->c_str();
                break;
            }
        }
        globalSymTable[targetTokenName] = targetSymbol;

        fprintf(outfile, "%s {%zd.%zd.%zd} ", targetTokenName.c_str(), targetSymbol->lloc.filenr, targetSymbol->lloc.linenr, targetSymbol->lloc.offset);
        if (targetSymbol->attributes[PTR] == 1) {
        fprintf(outfile, "struct %s ", targetSymbol->struct_name.c_str());
        }
        targetSymbol->dump(outfile);
        fprintf(outfile, "\n\n");
        
    } else if(strcmp(tokenName, "TOK_STRUCT") == 0) {
        
        astree* targetStructToken = tree->children[0];
        string targetStructName = targetStructToken->lexinfo->c_str();

        if (globalSymTable.find(targetStructName) != globalSymTable.end()) {
          fprintf(outfile, "Error\n");
          exit(EXIT_FAILURE);
        }

        symbol* targetStructSymbol = new_symbol(targetStructToken);
        targetStructSymbol->origin = tree;
        targetStructSymbol->attributes[STRUCT] = 1;
        globalSymTable[targetStructName] = targetStructSymbol;
        fprintf(outfile, "%s {%zd.%zd.%zd} ", targetStructName.c_str(), targetStructSymbol->lloc.filenr, targetStructSymbol->lloc.linenr, targetStructSymbol->lloc.offset);
        fprintf(outfile, "{%ld} ", targetStructSymbol->block_nr);
        targetStructSymbol->dump(outfile);
        fprintf(outfile, "%s", targetStructName.c_str());
        fprintf(outfile, "\n");

        for (unsigned int i = 1; i < tree->children.size(); i++) {
          astree* targetFieldToken = tree->children[i]->children[1];
          string targetFieldTokenName = targetFieldToken->lexinfo->c_str();

          astree* targetFieldTypeToken = tree->children[i]->children[0];

          //Checks if declaration already exists in symbol table
          if(globalSymTable.find(targetFieldTokenName) != globalSymTable.end()) {
            fprintf(outfile, "Error\n");
            exit(EXIT_FAILURE);
          }

          symbol* targetFieldSymbol = new_symbol(targetFieldToken);
          targetFieldSymbol->origin = tree;
          targetFieldSymbol->sequence = i - 1;
          targetFieldSymbol->attributes[FIELD] = 1;

          switch (targetFieldTypeToken->symbol) {
            case TOK_VOID : {
                targetFieldSymbol->attributes[VOID] = 1;
                break;
            }
            case TOK_INT : {
                targetFieldSymbol->attributes[INT] = 1;
                break;
            }
            case TOK_STRING : {
                targetFieldSymbol->attributes[STRING] = 1;
                targetFieldSymbol->attributes[LVAL] = 1;
                break;
            }
            case TOK_NULLPTR : {
                targetFieldSymbol->attributes[NULLPTR_T] = 1;
                targetFieldSymbol->attributes[CONST] = 1;
                break;
            }
            case TOK_ARRAY : {
                targetFieldSymbol->attributes[ARRAY] = 1;
                break;
            }

            case TOK_PTR : {
                targetFieldSymbol->attributes[PTR] = 1;
                targetFieldSymbol->struct_name = tree->children[i]->children[0]->children[0]->lexinfo->c_str();
                break;
            }

          }

        targetStructSymbol->fields[targetFieldTokenName] = targetFieldSymbol;
        fprintf(outfile, "  %s {%zd.%zd.%zd} ", targetFieldTokenName.c_str(), targetFieldSymbol->lloc.filenr, targetFieldSymbol->lloc.linenr, targetFieldSymbol->lloc.offset);
        if (targetFieldSymbol->attributes[PTR] == 1) {
        fprintf(outfile, "struct %s ", targetFieldSymbol->struct_name.c_str());
        }
        targetFieldSymbol->dump(outfile);
        fprintf(outfile, "%ld", targetFieldSymbol->sequence);
        fprintf(outfile, "\n");
      }
      fprintf(outfile, "\n");

    } else if(strcmp(tokenName, "TOK_FUNCTION") == 0) {
        
        astree* targetFunctionToken = tree->children[0]->children[1];
        string targetFunctionName = targetFunctionToken->lexinfo->c_str();

        if (localSymTable.find(targetFunctionName) != localSymTable.end()) {
          fprintf(outfile, "Error\n");
          exit(EXIT_FAILURE);
        }

        symbol* targetFunctionSymbol = new_symbol(targetFunctionToken);
        targetFunctionSymbol->origin = tree;
        targetFunctionSymbol->attributes[FUNCTION] = 1;
        targetFunctionSymbol->block_nr = blockNum;
        globalSymTable[targetFunctionName] = targetFunctionSymbol;
        fprintf(outfile, "%s {%zd.%zd.%zd} ", targetFunctionName.c_str(), targetFunctionSymbol->lloc.filenr, targetFunctionSymbol->lloc.linenr, targetFunctionSymbol->lloc.offset);
        fprintf(outfile, "{%ld} ", targetFunctionSymbol->block_nr - 1);
        if (tree->children[0]->children[0]->symbol == TOK_PTR) {
          fprintf(outfile, "ptr <struct %s> ", tree->children[0]->children[0]->children[0]->lexinfo->c_str());
        }
        else {
          fprintf(outfile, "%s ", tree->children[0]->children[0]->lexinfo->c_str());
        }
        targetFunctionSymbol->dump(outfile);
        fprintf(outfile, "\n");

        for (unsigned int i = 0; i < tree->children[1]->children.size(); i++) {
          astree* targetParamToken = tree->children[1]->children[i]->children[1];
          string targetParamTokenName = targetParamToken->lexinfo->c_str();

          astree* targetParamTypeToken = tree->children[1]->children[i]->children[0];

          symbol* targetParamSymbol = new_symbol(targetParamToken);
          targetParamSymbol->param_type = tree->children[1]->children[i]->children[0]->lexinfo->c_str();
          targetParamSymbol->param_name = tree->children[1]->children[i]->children[1]->lexinfo->c_str();
          targetParamSymbol->origin = tree;
          targetParamSymbol->sequence = i;
          targetParamSymbol->block_nr = blockNum;
          targetParamSymbol->attributes[PARAM] = 1;
          targetParamSymbol->attributes[VARIABLE] = 1;
          targetParamSymbol->attributes[LVAL] = 1;

          switch (targetParamTypeToken->symbol) {
            case TOK_VOID : {
                targetParamSymbol->attributes[VOID] = 1;
                break;
            }
            case TOK_INT : {
                targetParamSymbol->attributes[INT] = 1;
                targetParamSymbol->attributes[LVAL] = 1;
                break;
            }
            case TOK_STRING : {
                targetParamSymbol->attributes[STRING] = 1;
                targetParamSymbol->attributes[LVAL] = 1;
                break;
            }
            case TOK_NULLPTR : {
                targetParamSymbol->attributes[NULLPTR_T] = 1;
                targetParamSymbol->attributes[CONST] = 1;
                break;
            }
            case TOK_ARRAY : {
                targetParamSymbol->attributes[ARRAY] = 1;
                break;
            }

            case TOK_PTR : {
                targetParamSymbol->attributes[PTR] = 1;
                targetParamSymbol->struct_name = tree->children[i]->children[0]->children[0]->lexinfo->c_str();
                break;
            }
          }

        targetFunctionSymbol->parameters.push_back(targetParamSymbol);
        localSymTable[targetParamTokenName] = targetParamSymbol;
        fprintf(outfile, "  %s {%zd.%zd.%zd} ", targetParamTokenName.c_str(), targetParamSymbol->lloc.filenr, targetParamSymbol->lloc.linenr, targetParamSymbol->lloc.offset);
        fprintf(outfile, "{%ld} ", targetParamSymbol->block_nr);
        targetParamSymbol->dump(outfile);
        fprintf(outfile, "%ld", targetParamSymbol->sequence);
        fprintf(outfile, "\n");
      }

    if (tree->children.size() != 2) {
      for (unsigned int i = 0; i < tree->children[2]->children.size(); i++) {
        if (tree->children[2]->children[i]->children.size() == 1) {
          astree* targetReturnToken = tree->children[2]->children[i]->children[0];
          string targetReturnTokenName = targetReturnToken->lexinfo->c_str();

          symbol* returnBlockSymbol = new_symbol(targetReturnToken);
          fprintf(outfile, "  %s {%zd.%zd.%zd} ", targetReturnTokenName.c_str(), returnBlockSymbol->lloc.filenr, returnBlockSymbol->lloc.linenr, returnBlockSymbol->lloc.offset);
          fprintf(outfile, "{%ld} ", returnBlockSymbol->block_nr);
          returnBlockSymbol->dump(outfile);
          fprintf(outfile, "%ld", returnBlockSymbol->sequence);
          fprintf(outfile, "\n");
          break;
        }

        if (strcmp(parser::get_tname(tree->children[2]->children[i]->symbol), "TOK_TYPE_ID") != 0) {
          continue;
        }
          astree* targetBlockToken = tree->children[2]->children[i]->children[1];
          string targetBlockTokenName = targetBlockToken->lexinfo->c_str();
          if (targetBlockTokenName.compare("{") == 0) {
            continue;
          }

          astree* targetBlockTypeToken = tree->children[2]->children[i]->children[0];

          symbol* targetBlockSymbol = new_symbol(targetBlockToken);
          targetBlockSymbol->origin = tree;
          targetBlockSymbol->sequence = i;
          targetBlockSymbol->block_nr = blockNum;
          targetBlockSymbol->attributes[VARIABLE] = 1;
          targetBlockSymbol->attributes[LOCAL] = 1;
          targetBlockSymbol->attributes[LVAL] = 1;
          targetBlockSymbol->local_type = tree->children[2]->children[i]->children[0]->lexinfo->c_str();

          switch (targetBlockTypeToken->symbol) {
            case TOK_VOID : {
                targetBlockSymbol->attributes[VOID] = 1;
                break;
            }
            case TOK_INT : {
                targetBlockSymbol->attributes[INT] = 1;
                targetBlockSymbol->attributes[LVAL] = 1;
                break;
            }
            case TOK_STRING : {
                targetBlockSymbol->attributes[STRING] = 1;
                targetBlockSymbol->attributes[LVAL] = 1;
                targetBlockSymbol->string_name = tree->children[2]->children[i]->children[2]->lexinfo->c_str();
                break;
            }
            case TOK_NULLPTR : {
                targetBlockSymbol->attributes[NULLPTR_T] = 1;
                targetBlockSymbol->attributes[CONST] = 1;
                break;
            }
            case TOK_ARRAY : {
                targetBlockSymbol->attributes[ARRAY] = 1;
                break;
            }
            case TOK_PTR : {
              targetBlockSymbol->attributes[PTR] = 1;
              targetBlockSymbol->struct_name = tree->children[2]->children[i]->children[0]->children[0]->lexinfo->c_str();
            }
          }

        if (targetBlockSymbol->attributes[STRING] == 1) {
          globalSymTable[targetBlockTokenName] = targetBlockSymbol;
          localSymTable[targetBlockTokenName] = targetBlockSymbol;
        }
        else {
        localSymTable[targetBlockTokenName] = targetBlockSymbol;
        }
        fprintf(outfile, "  %s {%zd.%zd.%zd} ", targetBlockTokenName.c_str(), targetBlockSymbol->lloc.filenr, targetBlockSymbol->lloc.linenr, targetBlockSymbol->lloc.offset);
        fprintf(outfile, "{%ld} ", targetBlockSymbol->block_nr);
        if (targetBlockSymbol->attributes[PTR] == 1) {
          fprintf(outfile, "struct %s ", targetBlockSymbol->struct_name.c_str());
        }
        targetBlockSymbol->dump(outfile);
        fprintf(outfile, "%ld", targetBlockSymbol->sequence);
        fprintf(outfile, "\n");
      }
    }
      blockNum++;
      fprintf(outfile, "\n");
    } 
    else {
        //Recurses on all children from left to right if non symbol token reached
        for(unsigned int childNum = 0; childNum < tree->children.size(); childNum++) {
            crawl_tree(outfile, tree->children[childNum]);
        }
    }
}

symbol* new_symbol(astree* node) { //not for functions

  symbol* sym = new symbol();
  sym->lloc = node->lloc;
  
  sym->block_nr = 0;
  sym->sequence = 0;

  return sym;
  
}