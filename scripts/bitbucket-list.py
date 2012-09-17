import sys
import json

# example: curl -q -u ikh https://api.bitbucket.org/1.0/users/iridl | bitbucket-list.py | sh

s = ''.join(sys.stdin.readlines())
x = json.loads(s)

u = x['user']['username']
rs = x['repositories']
for r in rs:
  n = r['name']
  w = r['has_wiki']
  print("git-backup-repositories clone \"git@bitbucket.org:"+u+"/"+n+".git\" \""+u+"/"+n+".git\"")
  if w:
     print("git-backup-repositories clone \"git@bitbucket.org:"+u+"/"+n+".git/wiki\" \""+u+"/"+n+".wiki.git\"")
