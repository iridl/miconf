/*
 * Copyright (c) 2018 ikh software, inc. All rights reserved.
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
#include <python2.7/Python.h>

#include "version.h"

#define WRITE_BEG "'''"
#define WRITE_END "'''"

#define NL '\n'
#define SP ' '

#define doNewLine(c) fprintf(fo,"%*ssys.stdout.write('\\n') # %d\n",indent,"",lineno)

#define begText() fprintf(fo,"%*ssys.stdout.write(%s",indent,"",WRITE_BEG)
#define doText(c) (c=='\'' ? fprintf(fo,"\\'") : (c=='\\' ? fprintf(fo,"\\\\") :fputc(c,fo)))
#define endText() fprintf(fo,"%s) # %d\n",WRITE_END,lineno)

#define begStat() (indent=0, countIndent=1, fputc(NL,fo))
#define doStat(c) fputc(c,fo)
#define endStat() fprintf(fo," # %d\n",lineno)

#define begExpr() fprintf(fo,"%*ssys.stdout.write(str(",indent,"")
#define doExpr(c) fputc(c,fo)
#define endExpr() fprintf(fo,")) # %d\n", lineno)

void convert(FILE* fi, FILE* fo, int n, int eq, int la, int ra) {
   int lineno = 1;
   int c;
   int s = 1; //state
   int indent = 0;
   int countIndent = 1;
   do {
      c=getc(fi);
      switch (s) {
      case 1:
         switch (c) {
         case EOF: s=1; break;
         case NL: s=1; doNewLine(); break;
         default: 
            if (c==eq) {
               s=2;
            } else if(c==la) {
               s=6; begText();
            } else {
               s=5; begText(); doText(c); 
            }
            break;
         }
         break;
      case 2:
         switch (c) {
         case EOF: s=5; begText(); doText(eq); endText(); break;
         case NL: s=1; begText(); doText(eq); endText(); doNewLine(); break;
         default: 
            if (c==eq) {
               s=3; 
            } else {
               s=5; begText(); doText(eq); doText(c);
            }
            break;
         }
         break;
      case 3:
         switch (c) {
         case EOF: s=5; begText(); doText(eq); doText(eq); endText(); break;
         case NL: s=1; begText(); doText(eq); doText(eq); endText(); doNewLine(); break;
         default: 
            if (c==eq) {
               s=35;
            } else {
               s=5; begText(); doText(eq); doText(eq); doText(c);
            }
            break;
         }
         break;
      case 35:
         switch (c) {
         case EOF: s=5; begText(); doText(eq); doText(eq); doText(eq); endText(); break;
         case NL: s=1; begText(); doText(eq); doText(eq); doText(eq); endText(); doNewLine(); break;
         default:
            if (c==SP) {
               s=4; begStat();
            } else {
               s=5; begText(); doText(eq); doText(eq); doText(eq); doText(c);
            }
            break;
         }
         break;
      case 4:
         switch (c) {
         case EOF: s=1; endStat(); break;
         case NL: s=1; endStat(); break;
         default: 
            s=4; 
            if (countIndent && c==SP) {
               indent ++;
            } else {
               countIndent = 0;
            }
            doStat(c); 
            break;
         }
         break;
      case 5:
         switch (c) {
         case NL: s=1; endText(); doNewLine(); break;
         case EOF: s=5; endText(); break;
         default: 
            if (c==la) {
               s=6;
            } else {
               s=5; doText(c);
            }
            break;
         }
         break;
      case 6:
         switch (c) {
         case EOF: s=5; doText(la); endText(); break;
         case NL: s=1; doText(la); endText(); doNewLine(); break;
         default: 
            if (c==la) {
               s=7; 
            } else {
               s=5; doText(la); doText(c);
            }
            break;
         }
         break;
      case 7:
         switch (c) {
         case EOF: s=5; doText(la); doText(la); endText(); break;
         case NL: s=1; doText(la); doText(la); endText(); doNewLine(); break;
         default: 
            if (c==la) {
               s=8; endText(); begExpr();
            } else {
               s=5; doText(la); doText(la); doText(c);
            }
            break;
         }
         break;
      case 8:
         switch (c) {
         case EOF: s=8; endExpr(); break;
         default: 
            if (c==ra) {
               s=9;
            } else {
               s=8; doExpr(c);
            }
            break;
         }
         break;
      case 9:
         switch (c) {
         case EOF: s=8; doExpr(ra); endExpr(); break;
         default: 
            if (c==ra) {
               s=10; 
            } else {
               s=8; doExpr(ra); doExpr(c);
            }
            break;
         }
         break;
      case 10:
         switch (c) {
         case EOF: s=8; doExpr(ra); doExpr(ra); endExpr(); break;
         default: 
            if (c==ra) {
               s=5; endExpr(); begText();
            } else {
               s=8; doExpr(ra); doExpr(ra); doExpr(c);
            }
            break;
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

void process_file(const char* iname, const char* oname, int tflag, int mflag, int vflag, const char* pattern, int n, int cs, int cle, int cre) {

   if (vflag) {
      fprintf(stderr, "processing file '%s' -> '%s', (%d,'%c','%c','%c')\n", iname, oname, n,cs,cle,cre);
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
   fprintf(ft,"import sys\n");
   convert(fi,ft,n,cs,cle,cre);
   fclose(ft);

   if (!isStdin) {
      fclose(fi);
   }

   ft = fopen(tname,"r");
   if (ft) {
      if (PyRun_SimpleFile(ft, tname) != 0) {
         //fprintf(stderr,"%s (file='%s')\n",lua_tostring(L,-1),tname);
         exit(1);
      }
      fclose(ft);
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

void process_dir(const char* dname, int tflag, int mflag, int vflag, const char* pattern, const char* replace, int level) {
/*
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
         lua_getglobal(L,"piconf_fname_hook");
         lua_pushinteger(L,level);
         lua_pushstring(L,pattern);
         lua_pushstring(L,dname);
         lua_pushstring(L,dp->d_name);
         lua_pushinteger(L,dp->d_type);
         lua_pushstring(L,replace);
         if (lua_pcall(L, 6, 3, 0) != 0) {
            fprintf(stderr,"error running lua function 'piconf_fname_hook': %s\n", lua_tostring(L,-1));
            exit(1);
         }
         if (lua_istable(L,-1) && lua_isstring(L,-2) && lua_isstring(L,-3)) {
            const char* iname = lua_tostring(L,-3);
            const char* oname = lua_tostring(L,-2);

            lua_pushinteger(L,1);
            lua_gettable(L,-2);
            int n = lua_tointeger(L,-1);
            lua_pop(L,1);

            lua_pushinteger(L,2);
            lua_gettable(L,-2);
            int cs = lua_tointeger(L,-1);
            lua_pop(L,1);

            lua_pushinteger(L,3);
            lua_gettable(L,-2);
            int cle = lua_tointeger(L,-1);
            lua_pop(L,1);

            lua_pushinteger(L,4);
            lua_gettable(L,-2);
            int cre = lua_tointeger(L,-1);
            lua_pop(L,1);

            process_file(iname,oname,tflag,mflag,vflag,pattern,n,cs,cle,cre);
         }
         lua_pop(L,3);  
         break;
      case DT_DIR:
         if (strcmp(dp->d_name,".")==0 || strcmp(dp->d_name,"..")==0) 
            break;
         lua_getglobal(L,"piconf_dname_hook");
         lua_pushinteger(L,level);
         lua_pushstring(L,dname);
         lua_pushstring(L,dp->d_name);
         if (lua_pcall(L, 3, 1, 0) != 0) {
            fprintf(stderr,"error running lua function 'piconf_dname_hook': %s\n", lua_tostring(L,-1));
            exit(1);
         }
         if (lua_isstring(L,-1)) {
            process_dir(lua_tostring(L,-1),tflag,mflag,vflag,pattern,replace,level+1);
         }
         lua_pop(L,1);
         break;
      }
   }
   closedir(dirp);
*/
}

void version() {
      fprintf(stderr,"Piconf: configuration utility\n");
      fprintf(stderr,"Piconf %s Copyright (c) 2018 ikh software, inc.\n", miconf_VERSION );
}

void usage() {
      fprintf(stderr,"Piconf: configuration utility\n");
      fprintf(stderr,"Piconf %s Copyright (c) 2018 ikh software, inc.\n", miconf_VERSION );
      fprintf(stderr,"Commit: %s\n\n", miconf_VERSION_RAW );
      fprintf(stderr,"Usage:\n");
      fprintf(stderr,"   piconf [options] template_file output_file\n");
      fprintf(stderr,"   piconf [options] -r directory\n\n");
      fprintf(stderr,"Options:\n");
      fprintf(stderr,"   -c file -- config file, for example: -c config.lua \n");
      fprintf(stderr,"   -e block -- config block, for example: -e 'host=\"foo\"; ip=\"127.0.0.1\"' \n");
      fprintf(stderr,"   -p pattern -- template file name pattern (see 'piconf_fname_hook', default: '[.]template$')\n");
      fprintf(stderr,"   -s replace -- file name pattern replacement (see 'piconf_fname_hook', default: '')\n");
      fprintf(stderr,"   -t -- preserve temp files\n");
      fprintf(stderr,"   -m -- disable chmod\n");
      fprintf(stderr,"   -v -- verbose\n");
      fprintf(stderr,"   -V -- version\n");
      fprintf(stderr,"   -h -- help\n\n");
}


int main(int argc, char* argv[]) {
   int ch;
   int rflag = 0;
   int tflag = 0;
   int mflag = 0;
   int vflag = 0;
   const char* pattern = "[.]template$";
   const char* replace = "";

   Py_SetProgramName(argv[0]);
   Py_Initialize();

/*
   const char* hooks = 
      "function piconf_dname_hook(level,path,file)\n"
      "   if file == \".git\" then\n"
      "      return nil\n"
      "   else\n"
      "      return path..(file and (\"/\"..file) or \"\")\n"
      "   end\n"
      "end\n"
      "function piconf_markup_hook()\n"
      "   return {3,string.byte(\"=\"),string.byte(\"<\"),string.byte(\">\")}\n"
      "end\n"
      "function piconf_fname_hook(level,pattern,path,file,type,replace)\n"
      "   ofile,cnt = file:gsub(pattern,replace,1)\n"
      "   if ofile and cnt==1 and ofile:len()>0 then\n"
      "      return path..(file and (\"/\"..file) or \"\"), path..\"/\"..ofile, piconf_markup_hook()\n"
      "   else\n"
      "      return nil,nil,nil\n"
      "   end\n"
      "end\n"
   ;

   if (luaL_dostring(L,hooks)!=0) {
      fprintf(stderr,"%s (string='%s')\n",lua_tostring(L,-1),hooks);
      lua_pop(L,1);
      exit(1);
   }
*/

   while ((ch = getopt(argc, argv, "hVrtmvp:s:e:c:")) != -1) {
      FILE *f;
      switch (ch) {
      case 'V':
         version();
         exit(0);
      case 'h':
         usage();
         fprintf(stderr,"Default config:\n\n");
         //fprintf(stderr,"%s\n",hooks);
         fprintf(stderr,"----\n\n");
         exit(0);
      case 'r': rflag = 1; break;
      case 't': tflag = 1; break;
      case 'm': mflag = 1; break;
      case 'v': vflag = 1; break;
      case 'p': pattern = optarg; break;
      case 's': replace = optarg; break;
      case 'c':
         f = fopen(optarg,"r");
         if (f) {
            if (PyRun_SimpleFile(f, optarg) != 0) {
               //fprintf(stderr,"%s (file='%s')\n",lua_tostring(L,-1),optarg);
               exit(1);
            }
            fclose(f);
         }
         break;
      case 'e':
         if (PyRun_SimpleString(optarg)!=0) {
            //fprintf(stderr,"%s (block='%s')\n",lua_tostring(L,-1),optarg);
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

      int n = 3;
      int cs = '=';
      int cle = '<';
      int cre = '>';
/*
      lua_getglobal(L,"piconf_markup_hook");
      if (lua_pcall(L, 0, 1, 0) != 0) {
         fprintf(stderr,"error running lua function 'piconf_markup_hook': %s\n", lua_tostring(L,-1));
         exit(1);
      }

      if (!lua_istable(L,-1)) {
         fprintf(stderr,"'piconf_markup_hook' must return a table\n");
         exit(1);
      }


      lua_pushinteger(L,1);
      lua_gettable(L,-2);
      int n = lua_tointeger(L,-1);
      lua_pop(L,1);

      lua_pushinteger(L,2);
      lua_gettable(L,-2);
      int cs = lua_tointeger(L,-1);
      lua_pop(L,1);

      lua_pushinteger(L,3);
      lua_gettable(L,-2);
      int cle = lua_tointeger(L,-1);
      lua_pop(L,1);

      lua_pushinteger(L,4);
      lua_gettable(L,-2);
      int cre = lua_tointeger(L,-1);
      lua_pop(L,1);
*/

      process_file(iname,oname,tflag,mflag,vflag,pattern,n,cs,cle,cre);

/*
      lua_pop(L,1);
*/
   } else {
/*
      const char* dname = argc>0 ? argv[0] : ".";
      lua_getglobal(L,"piconf_dname_hook");
      lua_pushinteger(L,0);
      lua_pushstring(L,dname);
      lua_pushnil(L);
      if (lua_pcall(L, 3, 1, 0) != 0) {
         fprintf(stderr,"error running lua function 'piconf_dname_hook': %s\n", lua_tostring(L,-1));
         exit(1);
      }
      if (lua_isstring(L,-1)) {
         process_dir(lua_tostring(L,-1),tflag,mflag,vflag,pattern,replace,1);
      }
      lua_pop(L,1);
*/
   }

/*
   lua_close(L);
*/

   return(0);
}

