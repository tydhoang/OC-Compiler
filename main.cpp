/*
* Created by:
*  Tyler Hoang (tydhoang@ucsc.edu)
*  Eric Vin (evin@ucsc.edu)
* main.cpp
*/
using namespace std;

#include "main.h"
#include <string>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_set.h"
#include <stdbool.h>
#include <getopt.h>
#include <vector>
#include <iostream>
#include <errno.h>
#include <libgen.h>
#include "lyutils.h"

#include "symtable.h"
#include "oilconvert.h"

FILE* tokOutputFile;
FILE* oilOutputFile;

int main (int argc, char** argv) {
   const int LINESIZE = 1024;

   // declare and initialize flag variables
   string preprocessorParams = "";
   yy_flex_debug = 0; // used for flag l
   yydebug = 0; // used for flag y

   int flag;
   while ((flag = getopt(argc, argv, "@:D:ly")) != -1) {
      switch (flag) {
         case '@' :
            // perform operation
            break;
         case 'D' :
            preprocessorParams = optarg;
            break;
         case 'l' :
            yy_flex_debug = 1;
            break;
         case 'y' :
            yydebug = 1;
            break;
         default :
            badUsage(); // illegal input
            break;
      }
   }

   // Retrieves last argument (Target file name)
   string targetFileName = argv[argc-1];

   // Checks that file name ends in ".oc"
   if(targetFileName.substr(targetFileName.size()-3).
compare(".oc") != 0) {
      badUsage(); // illegal input
   }

   // Attempts to open file
   FILE* targetFile = fopen(targetFileName.c_str(), "r");

   // Checks to ensure file exists and was successfully opened
   if(targetFile == NULL) {
      std::cerr << "File Read Error: " << errno 
<< "  Cannot open " << targetFileName  << std::endl;
      exit(EXIT_FAILURE);
   }
   fclose(targetFile);

   // Combines c preprocessor command with passed arguments
   string cppCommand = "/usr/bin/cpp -nostdinc";
 
   if(preprocessorParams.compare("") != 0) {
      cppCommand = cppCommand + " -" + preprocessorParams;
   }
   cppCommand = cppCommand + " " + targetFileName; 

   FILE* pipe = popen (cppCommand.c_str(), "r");

   // Preprocesses the input file using cpp
   string preprocessedInput = "";
   char buffer[LINESIZE]; // buffer used for each line of the input file
   char* rc; // points to each line in the input file

   int filledFlag = 0;
   do {
      if (filledFlag && buffer[0] != '#') { 
         preprocessedInput = preprocessedInput + buffer;
      }

      rc = fgets (buffer, 1024, pipe);

      // Triggers filledFlag
      filledFlag = 1;

   }  while (rc != nullptr); // while there are lines to be read in
   pclose(pipe); // close the pipe

   // Tokenizes the preprocessed input
   char* convertedPreprocessedInput = 
     new char[preprocessedInput.size() + 1];
   strcpy(convertedPreprocessedInput, preprocessedInput.c_str());

   vector<string> tokens;

   // extracted from provided cppstrtoke.cpp
   char* bufptr = convertedPreprocessedInput;
   char* savepos = nullptr;
   for (int tokenct = 1;; ++tokenct) {
      char* token = strtok_r (bufptr, " \t\n", &savepos);
      bufptr = nullptr;
      if (token == nullptr) break;
      tokens.push_back(token); // add the token to tokens vector
   }
   delete [] convertedPreprocessedInput;

   // Adds tokens hash table and prints to output file
   for (std::vector<string>::size_type i = 0; i < tokens.size(); ++i) {
      string_set::intern (tokens[i].c_str());
   }

   FILE* strOutputFile = fopen((targetFileName.substr
(0, targetFileName.size()-3) + ".str").c_str(), "w");

   string_set::dump (strOutputFile); // dump to the output file
  
   fclose(strOutputFile); // close the output file

   yyin = popen (cppCommand.c_str(), "r");

   tokOutputFile = fopen((targetFileName.substr
(0, targetFileName.size()-3) + ".tok").c_str(), "w");

   oilOutputFile = fopen((targetFileName.substr
(0, targetFileName.size()-3) + ".oil").c_str(), "w");

   FILE* symOutputFile = fopen((targetFileName.substr
(0, targetFileName.size()-3) + ".sym").c_str(), "w");

FILE* astOutputFile = fopen((targetFileName.substr
(0, targetFileName.size()-3) + ".ast").c_str(), "w");

   parser::root = new astree (TOK_ROOT, {0, 0, 0}, "");
   int yy = yyparse();

   if (yy != 0) {
      return EXIT_FAILURE;
   }
   
   //SYMBOL TABLE
   make_symbol_table(symOutputFile, parser::root);
   outputOil(oilOutputFile);

   astree::print(astOutputFile, parser::root);
   fclose(tokOutputFile); // close the output file
   fclose(symOutputFile);
   fclose(astOutputFile);
   fclose(oilOutputFile);
   pclose(yyin);

   delete parser::root;
   yylex_destroy();

   return EXIT_SUCCESS;
}

/**
  * badUsage method that is used for illegal inputs
  * 
*/
void badUsage() {
   std::cerr << "Usage: oc [-ly] [-@ flag . . .] [-D string] program.oc"
<< std::endl;
   exit(EXIT_FAILURE);
}