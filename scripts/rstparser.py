import sys
import os
import re

def err(s):
   sys.stderr.write("error: "+s+"\n")

def fixDirective(d):
   if "args" not in d:
      d["args"] = []
   if "opts" not in d:
      d["opts"] = []
   if "lines" not in d:
      d["lines"] = []
   dirpath = os.path.dirname(d["filepath"])
   while len(d["lines"])!=0 and re.match("^\s*$",d["lines"][len(d["lines"])-1]):
      d["lines"].pop()
   p1 = re.compile("^.*\.rst$")
   t = d["type"]
   files = []
   directives = []
   if t == "toctree":
      for line in d["lines"]:
         if not re.match("^\s*$",line):
            m1 = p1.match(line)
            fn = os.path.join(dirpath,line.strip() if m1 else (line.strip()+".rst"))
            files.append(fn)
            if os.path.isfile(fn):
               ds = rstparser(fn)
            else:
               ds = []
               err( "file '%s' used in directive '%s' in file %s:%d does not exist" % (fn,d["type"],d["filepath"],d["lineno"]) )
            directives += ds
   elif t == "figure" or t == "image":
      files.append(os.path.join(dirpath,d["args"][0].strip()))
   elif t == "csv-table":
      if "file" in d["opts"]:
         d["opts"]["file"] = d["opts"]["file"].strip()
         files.append(os.path.join(dirpath,d["opts"]["file"]))
   d["files"] = files
   d["directives"] = directives
   return d
   

def rstparser(filepath):
   f = open(filepath)
   p1 = re.compile("^\.\. ([A-Za-z-_+:.]+)::\s+(.*)$")
   p2 = re.compile("^   (:([A-Za-z-_+.]+):\s*(.*))$")
   p3 = re.compile("^\s*$")
   p4 = re.compile("^   (.*)$")
   ds = []
   d = None
   lineno = 0
   for line in f:
      lineno += 1
      m1 = p1.match(line)
      m2 = p2.match(line)
      m3 = p3.match(line)
      m4 = p4.match(line)
      if m1:
         if d:
            ds.append(fixDirective(d))
            d = None
         d = dict( filepath=filepath, lineno=lineno, type=m1.group(1).lower(), args=re.split("\s+",m1.group(2)), opts=dict() )
      elif d and not "lines" in d:
         if m2:
            if not m2.group(2) in d["opts"]:
               d["opts"][m2.group(2)] = m2.group(3)
            else:
               err( "duplicate option '%s' = '%s' in directive '%s' in file %s:%d" % (m2.group(2),m2.group(3),d["type"],d["filepath"],d["lineno"]) )
         elif m3:
            d["lines"] = []
         else:
            d["lines"] = []
            ds.append(fixDirective(d))
            d = None
      elif d and "lines" in d:
         if m2: 
            d["lines"].append(m2.group(1))
         elif m3 or m4:
            d["lines"].append(m4.group(1) if m4 else "")
         else:
            ds.append(fixDirective(d))
            d = None
      else:
         if d:
            ds.append(d)
            d = None
   if d:
      ds.append(fixDirective(d))
      d = None
   f.close()
   return ds

def printDirectives(ds,level):
   for d in ds:
      sys.stdout.write( "%sD %s:%d  %s  (args:%d opts:%d, lines:%d, files:%d, directives:%d)\n" % ("   "*level,d["filepath"],d["lineno"],d["type"],len(d["args"]),len(d["opts"]),len(d["lines"]),len(d["files"]),len(d["directives"])))
      for fpath in d["files"]:
         if os.path.isfile(fpath):
            sys.stdout.write( "%sF %s" % ("   "*level*2,fpath) )
         else:
            err( "file '%s' used in directive '%s' in %s:%d does not exist" % (fpath,d["type"],d["filepath"],d["lineno"]) )

      printDirectives(d["directives"],level+1)

ds = []
for fn in sys.argv:
   if os.path.isfile(fn):
      ds += rstparser(fn)
   else:
      err( "file '%s' does not exist (skipped)" % (fn,) )
printDirectives(ds,0)

