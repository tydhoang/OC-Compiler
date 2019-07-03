// $Id: oclib.c,v 1.6 2019-04-24 17:46:41-07 - - $

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define not !
#define nullptr 0
#define string char*

#include "oclib.h"

void fail (string expr, string file, int line) {
   fprintf (stderr, "%s:%d: assert (%s) failed\n", file, line, expr);
   abort();
}

void* xcalloc (int nelem, int size) {
   void* result = calloc (nelem, size);
   assert (result != nullptr);
   return result;
}

void putchr (int chr) { printf ("%c", chr); }
void putint (int num) { printf ("%d", num); }
void putstr (string str) { printf ("%s", str); }

int getchr() { return getchar(); }

string getstr (void) {
   char buffer[0x1000];
   char format[16];
   sprintf (format, "%%%zds", sizeof buffer - 1);
   int count = scanf (format, buffer);
   return count != 1 ? nullptr : strdup (buffer);
}

string getln (void) {
   char buffer[0x1000];
   string result = fgets (buffer, sizeof buffer, stdin);
   return result == nullptr ? nullptr : strdup (result);
} 

