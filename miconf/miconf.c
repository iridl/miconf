/*
 * Copyright (c) 2012 ikh software, inc. All rights reserved.
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
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "version"

#define WRITE_BATCH 50
#define WRITE_BEG "[=====["
#define WRITE_END "]=====]"

#define EQ '='
#define LA '<'
#define RA '>'
#define NL '\n'

#define doNewLine(c) fprintf(fo,"io.write('\\n') -- %d\n",lineno)

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
         case NL: s=1; doNewLine(); break;
         default: s=5; begText(); doText(c); break;
         }
         break;
      case 2:
         switch (c) {
         case EQ: s=3; break;
         case EOF: s=5; doText(EQ); endText(); break;
         case NL: s=1; doText(EQ); endText(); break;
         default: s=5; doText(EQ); doText(c); break;
         }
         break;
      case 3:
         switch (c) {
         case EQ: s=4; begStat(); break;
         case EOF: s=5; doText(EQ); doText(EQ); endText(); break;
         case NL: s=1; doText(EQ); doText(EQ); endText(); break;
         default: s=5; doText(EQ); doText(EQ); doText(c); break;
         }
         break;
      case 4:
         switch (c) {
         case EOF: s=1; endStat(); break;
         case NL: s=1; endStat(); break;
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
         case NL: s=1; doText(LA); endText(); break;
         default: s=5; doText(LA); doText(c); break;
         }
         break;
      case 7:
         switch (c) {
         case LA: s=8; endText(); begExpr(); break;
         case EOF: s=5; doText(LA); doText(LA); endText(); break;
         case NL: s=1; doText(LA); doText(LA); endText(); break;
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
      fprintf(stderr,"file read error (errno=%d: %s)\n", errno, strerror(errno));
      exit(1);
   }
}

void process_file(lua_State *L, const char* iname, const char* oname, int tflag, int mflag, int vflag, const char* pattern) {

   if (vflag) {
      fprintf(stderr, "processing file '%s' -> '%s'\n", iname, oname);
   }

   const char* tname;
   FILE* fi;
   FILE* ft;
   struct stat tstats;
   int isStdin = (strcmp(iname,"-")==0);
   int isStdout = (strcmp(oname,"-")==0);

   mflag = mflag || isStdin || isStdout;

   if (isStdin) {
      fi = stdin;
   } else {
      fi = fopen(iname,"r");
      if (fi==NULL) {
         fprintf(stderr,"can't open input file (file='%s', errno=%d: %s)\n", iname, errno, strerror(errno));
         exit(1);
      }
   }

   if (!isStdout) {
      stdout = freopen(oname, "w", stdout);
      if (stdout==NULL) {
         fprintf(stderr,"can't open output file (file='%s', errno=%d: %s)\n", oname, errno, strerror(errno));
         exit(1);
      }
   }

   tname = tmpnam(NULL);
   if (tname==NULL) {
      fprintf(stderr,"can't obtain temporary file name (errno=%d: %s)\n", errno, strerror(errno));
      exit(1);
   }
   ft = fopen(tname,"w");
   if (ft==NULL) {
      fprintf(stderr,"can't open temporary file (file='%s', errno=%d: %s)\n", tname, errno, strerror(errno));
      exit(1);
   }
   convert(fi,ft);
   fclose(ft);

   if (!isStdin) {
      fclose(fi);
   }

   if (luaL_dofile(L,tname)!=0) {
      fprintf(stderr,"%s (file='%s')\n",lua_tostring(L,-1),tname);
      lua_pop(L,1);
      exit(1);
   }

//   if (!isStdout) {
//        stdout = freopen("/dev/null", "w", stdout);
//        if (stdout==NULL) {
//           fprintf(stderr,"can't open /dev/null (errno=%d: %s)\n", errno, strerror(errno));
//           exit(1);
//        }
//   }

   if (!mflag) {
      if (stat(iname,&tstats) != 0) {
         fprintf(stderr,"can't stat input file (file='%s', errno=%d: %s)\n", iname, errno, strerror(errno));
         exit(1);
      }
      if (chmod(oname,tstats.st_mode)!=0) {
         fprintf(stderr,"can't chmod output file (file='%s', mode=%o, errno=%d: %s)\n", tname, tstats.st_mode, errno, strerror(errno));
         exit(1);
      }
   }

   if (!tflag) {
      if (unlink(tname)!=0) {
         fprintf(stderr,"can't remove temporary file (file='%s', errno=%d: %s)\n", tname, errno, strerror(errno));
         exit(1);
      }
   } else if (vflag){
      fprintf(stderr,"temporary file '%s'\n",tname); 
   }
}

void process_dir(lua_State *L, const char* dname, int tflag, int mflag, int vflag, const char* pattern, int level) {

   if (vflag) {
      fprintf(stderr, "processing dir(%d) '%s'\n", level, dname);
   }

   DIR *dirp = opendir(dname);
   if (dirp==NULL) {
      fprintf(stderr,"can't open directory (name='%s', errno=%d: %s)\n", dname, errno, strerror(errno));
      exit(1);
   }
   struct dirent* dp;
   while ((dp=readdir(dirp)) != NULL) {
      switch(dp->d_type) {
      case DT_REG:
         lua_getglobal(L,"miconf_fname_hook");
         lua_pushinteger(L,level);
         lua_pushstring(L,pattern);
         lua_pushstring(L,dname);
         lua_pushstring(L,dp->d_name);
         lua_pushinteger(L,dp->d_type);
         if (lua_pcall(L, 5, 2, 0) != 0) {
            fprintf(stderr,"error running lua function 'miconf_fname_hook': %s\n", lua_tostring(L,-1));
            exit(1);
         }
         if (lua_isstring(L,-1) && lua_isstring(L,-1)) {
            const char* iname = lua_tostring(L,-2);
            const char* oname = lua_tostring(L,-1);
            process_file(L,iname,oname,tflag,mflag,vflag,pattern);
         }
         lua_pop(L,2);  
         break;
      case DT_DIR:
         if (strcmp(dp->d_name,".")==0 || strcmp(dp->d_name,"..")==0) 
            break;
         lua_getglobal(L,"miconf_dname_hook");
         lua_pushinteger(L,level);
         lua_pushstring(L,dname);
         lua_pushstring(L,dp->d_name);
         if (lua_pcall(L, 3, 1, 0) != 0) {
            fprintf(stderr,"error running lua function 'miconf_dname_hook': %s\n", lua_tostring(L,-1));
            exit(1);
         }
         if (lua_isstring(L,-1)) {
            process_dir(L,lua_tostring(L,-1),tflag,mflag,vflag,pattern,level+1);
         }
         lua_pop(L,1);
         break;
      }
   }
   closedir(dirp);

}


void usage() {
      fprintf(stderr,"Miconf: configuration utility\n\n");
      fprintf(stderr,"Miconf %s Copyright (c) 2012 ikh software, inc.\n", strchr(MICONF_RELEASE,'-')+1);
      fprintf(stderr,"%s\n\n", LUA_COPYRIGHT);
      fprintf(stderr,"Usage:\n");
      fprintf(stderr,"   miconf [options] template_file output_file\n");
      fprintf(stderr,"   miconf [options] -r directory\n\n");
      fprintf(stderr,"Options:\n");
      fprintf(stderr,"   -c file -- config file, for example: -c config.lua \n");
      fprintf(stderr,"   -e block -- config block, for example: -e 'host=\"foo\"; ip=\"127.0.0.1\"' \n");
      fprintf(stderr,"   -p pattern -- template file name pattern (default: '[.]template$')\n");
      fprintf(stderr,"   -t -- preserve temp files\n");
      fprintf(stderr,"   -m -- disable chmod\n");
      fprintf(stderr,"   -v -- verbose\n");
      fprintf(stderr,"   -h -- help\n\n");
}


int main(int argc, char* argv[]) {
   int ch;
   int rflag = 0;
   int tflag = 0;
   int mflag = 0;
   int vflag = 0;
   const char* pattern = "[.]template$";
   lua_State *L = luaL_newstate();
   luaL_openlibs(L);


   const char* hooks = 
      "function miconf_dname_hook(level,path,file)\n"
      "   return path..(file and (\"/\"..file) or \"\")\n"
      "end\n"
      "function miconf_fname_hook(level,pattern,path,file,type)\n"
      "   ofile,cnt = file:gsub(pattern,\"\")\n"
      "   if ofile and cnt==1 and ofile:len()>0 then\n"
      "      return path..(file and (\"/\"..file) or \"\"), path..\"/\"..ofile\n"
      "   else\n"
      "      return nil,nil\n"
      "   end\n"
      "end\n"
   ;

   if (luaL_dostring(L,hooks)!=0) {
      fprintf(stderr,"%s (string='%s')\n",lua_tostring(L,-1),hooks);
      lua_pop(L,1);
      exit(1);
   }

   while ((ch = getopt(argc, argv, "hrtmvp:e:c:")) != -1) {
      switch (ch) {
      case 'h':
         usage();
         fprintf(stderr,"Default config:\n\n",hooks);
         fprintf(stderr,"%s\n",hooks);
         fprintf(stderr,"----\n\n");
         exit(0);
      case 'r': rflag = 1; break;
      case 't': tflag = 1; break;
      case 'm': mflag = 1; break;
      case 'v': vflag = 1; break;
      case 'p': pattern = optarg; break;
      case 'c':
         if (luaL_dofile(L,optarg)!=0) {
            fprintf(stderr,"%s (file='%s')\n",lua_tostring(L,-1),optarg);
            lua_pop(L,1);
            exit(1);
         }
         break;
      case 'e':
         if (luaL_dostring(L,optarg)!=0) {
            fprintf(stderr,"%s (block='%s')\n",lua_tostring(L,-1),optarg);
            lua_pop(L,1);
            exit(1);
         }
         break;
      default:
         usage();
         exit(1);
      }
   }
   argc -= optind;
   argv += optind;

   if (!rflag) {
      const char* iname = argc>0 ? argv[0] : "-";
      const char* oname = argc>1 ? argv[1] : "-";
      process_file(L,iname,oname,tflag,mflag,vflag,pattern);
   } else {
      const char* dname = argc>0 ? argv[0] : ".";
      lua_getglobal(L,"miconf_dname_hook");
      lua_pushinteger(L,0);
      lua_pushstring(L,dname);
      lua_pushnil(L);
      if (lua_pcall(L, 3, 1, 0) != 0) {
         fprintf(stderr,"error running lua function 'miconf_dname_hook': %s\n", lua_tostring(L,-1));
         exit(1);
      }
      if (lua_isstring(L,-1)) {
         process_dir(L,lua_tostring(L,-1),tflag,mflag,vflag,pattern,1);
      }
      lua_pop(L,1);
   }

   lua_close(L);

   return(0);
}

