/*
 * Copyright (c) 2012 Igor Khomyakov. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1) Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2) Redistributions of source code with modification must include a notice
 * that the code was modified.
 * 3) Neither the names of the authors nor the names of their contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#define TMPFNAME "tmp.lua"

#define WRITE_BATCH 50
#define WRITE_BEG "[=====["
#define WRITE_END "]=====]"

#define EQ '='
#define LA '<'
#define RA '>'
#define NL '\n'

#define begText() fprintf(fo,"io.write(%s",WRITE_BEG)
#define doText(c) fputc(c,fo)
#define endText() fprintf(fo,"%s) -- %d\n",WRITE_END,lineno)

#define begStat() fputc(NL,fo)
#define doStat(c) fputc(c,fo)
#define endStat() fprintf(fo," -- %d\n",lineno)

#define begExpr() fprintf(fo,"io.write(tostring(")
#define doExpr(c) fputc(c,fo)
#define endExpr() fprintf(fo,")) -- %d\n", lineno)

//         if (lineno % WRITE_BATCH == 0) {

void convert(FILE* fi, FILE* fo) {
   int lineno = 1;
   int c;
   int s = 1; //state
   do {
      c=getc(fi);
      switch (s) {
      case 1:
         switch (c) {
         case EQ: s=2; break;
         case LA: s=6; break;
         case EOF: s=1; break;
         default: s=5; begText(); doText(c); break;
         }
         break;
      case 2:
         switch (c) {
         case EQ: s=3; break;
         case EOF: s=5; doText(EQ); endText(); break;
         default: s=5; doText(EQ); doText(c); break;
         }
         break;
      case 3:
         switch (c) {
         case EQ: s=4; begStat(); break;
         case EOF: s=5; doText(EQ); doText(EQ); endText(); break;
         default: s=5; doText(EQ); doText(EQ); doText(c); break;
         }
         break;
      case 4:
         switch (c) {
         case NL: s=1; endStat(); break;
         case EOF: s=1; endStat(); break;
         default: s=4; doStat(c); break;
         }
         break;
      case 5:
         switch (c) {
         case LA: s=6; break;
         case NL: s=1; doText(c); endText(); break;
         case EOF: s=5; endText(); break;
         default: s=5; doText(c); break;
         }
         break;
      case 6:
         switch (c) {
         case LA: s=7; break;
         case EOF: s=5; doText(LA); endText(); break;
         default: s=5; doText(LA); doText(c); break;
         }
         break;
      case 7:
         switch (c) {
         case LA: s=8; endText(); begExpr(); break;
         case EOF: s=5; doText(LA); doText(LA); endText(); break;
         default: s=5; doText(LA); doText(LA); doText(c); break;
         }
         break;
      case 8:
         switch (c) {
         case RA: s=9; break;
         case EOF: s=8; endExpr(); break;
         default: s=8; doExpr(c); break;
         }
         break;
      case 9:
         switch (c) {
         case RA: s=10; break;
         case EOF: s=8; doExpr(RA); endExpr(); break;
         default: s=8; doExpr(RA); doExpr(c); break;
         }
         break;
      case 10:
         switch (c) {
         case RA: s=5; endExpr(); begText(); break;
         case EOF: s=8; doExpr(RA); doExpr(RA); endExpr(); break;
         default: s=8; doExpr(RA); doExpr(RA); doExpr(c); break;
         }
         break;
      } 
      if (c==NL) {
         lineno++;
      }

   } while (c!=EOF);

   if (ferror(fi)) {
      fprintf(stderr,"file read error (errno=%d)\n", errno);
      exit(1);
   }
}


int main(int argc, char* argv[]) {
   const char* cname = argc>1 ? argv[1] : "config.lua";
   const char* tname = argc>2 ? argv[2] : "-";
   FILE* fi;
   FILE* fo;
   lua_State *L = luaL_newstate();
   luaL_openlibs(L);
   if (luaL_dofile(L,cname)!=0) {
      fprintf(stderr,"%s (file=%s)\n",lua_tostring(L,-1),cname);
      lua_pop(L,1);
      exit(1);
   }
   if (strcmp(tname,"-")!=0) {
      fi = fopen(tname,"r");
      if (fi==NULL) {
         fprintf(stderr,"can't open input file (file=%s, errno=%d)\n", tname, errno);
         exit(1);
      }
   } else {
      fi = stdin;
   }

   fo = fopen(TMPFNAME,"w");
   if (fo==NULL) {
      fprintf(stderr,"can't open temporary file (file=%s, errno=%d)\n", TMPFNAME, errno);
      exit(1);
   }
   convert(fi,fo);
   fclose(fo);

   if (strcmp(tname,"-")!=0) {
      fclose(fi);
   }

   if (luaL_dofile(L,TMPFNAME)!=0) {
      fprintf(stderr,"%s (file=%s)\n",lua_tostring(L,-1),TMPFNAME);
      lua_pop(L,1);
      exit(1);
   }

   lua_close(L);

   return(0);
}

