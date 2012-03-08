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

#define WRITE_BATCH 50
#define WRITE_BEG "[=====["
#define WRITE_END "]=====]"

void convert(FILE* fi, FILE* fo) {
   int lineno = 1;
   int c;
   fprintf(fo,"io.write(%s",WRITE_BEG);
   do {
      c = getc(fi);
      if (c == EOF) {
         if (ferror(fi)) {
            fprintf(stderr,"stdin error: %d\n", errno);
            exit(1);
         }
      } else if (c == '\n') {
         fputc(c,fo);
         if (lineno % WRITE_BATCH == 0) {
            fprintf(fo,"%s)\nio.write(%s",WRITE_END,WRITE_BEG);
         }
         lineno++;
      } else {
         fputc(c,fo);
      }
   } while (c != EOF);
   fprintf(fo,"%s)\n",WRITE_END);
}


int main(int argc, char* argv[]) {
   lua_State *L = luaL_newstate();
   luaL_openlibs(L);
   if (luaL_dofile(L,"config.lua")!=0) {
      fprintf(stderr,"%s\n",lua_tostring(L,-1));
      lua_pop(L,1);
      exit(1);
   }
   convert(stdin,stdout);
   lua_close(L);
   return(0);
}

