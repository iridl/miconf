#piconf
##Name
`piconf` - lightweight configuration utility with Python 
##Synopsis
	piconf -h
	piconf [options] -r directory
	piconf [options] template_file output_file
##Description
`piconf` is a lightweight configuration utility based on simple template substitution technique and [Python](https://docs.python.org) expressions. It executes a configuration file (a Python script), and then uses the results of the execution (its variable state) in template substitution. It reads `template_file` and substitutes `<<<`*Python expression*`>>>` placeholders with evaluated *Python expression*. It also executes lines with Python statements that start with `===`. 

For example:

        . . .
        === x = 3
        <<<x>>> ^ 3 = <<< x^3 >>>
        . . .

will result in the following output:

        . . .
        3 ^ 3 = 27
        . . .

Python uses whitespace (indentation) to define program blocks. The templates assume the current indentation. If you want to increase or decrease the amount of indentation, use `pass` statement with proper indentation.


For example:

	. . .
	=== for x in range(1,3):
	===    for y in range(1,3):
        ===       pass
        <<<x>>> * <<<y>>> = <<< x * y >>>
        ===    pass
        x = <<< x >>>
        === pass
        chao pescado
	. . .

will result in the following output:

	. . .
	1 * 1 = 1
	1 * 2 = 2
        x = 1
	2 * 1 = 2
	2 * 2 = 4
        x = 2
        chao pescado
	. . .

		
When `piconf` is run in recoursive `-r` mode, it traverses `directory` recoursively and processes files based on the pattern (`-p` option) and the output of callback functions.

##Options

| Option 	| Default value 	| Description 	| Example 	| 
| :--- 	| :--- 	| :--- 	| :--- 	|
| `-c file` 	| `'config.py'` 	| config file 	| `-c system.config` 	|
| `-e block` 	| `''` 	| config block 	| `-e 'host="foo"; ip="127.0.0.1"'`	|
| `-p pattern` 	| `'[.]template$'` 	| template file name pattern (see `piconf_fname_hook`) 	| `-p '[.]piconf([.]?)'` 	|
| `-s replace` 	| `''` 	| file name pattern replacement (see `piconf_fname_hook`) 	| `-s '%1'` 	|
| `-t` 	| 	| preserve temp files 	| 	|
| `-m` 	| 	| disable chmod 	| 	|
| `-v` 	| 	| verbose 	| 	|

##Default hooks

The following "hooks" are used if you do not redefine them in your configuration files. 
These functions get called when `piconf` is run in recoursive mode using `-r` option. `piconf_dname_hook` is invoked for each subdirectory, and `piconf_fname_hook` is called for each regular file. `piconf_dname_hook` has to return a full path to the subdirectory to traverse, or `nil` to ignore the whole subdirectory. `piconf_fname_hook` has to return input file path, output file path and markup syntax description table (see `piconf_markup_hook` below), or `(nil,nil,nil)` if you want to ignore the file. These functions allow for customization of piconf's behavior, i.e. what parts of your tree and what files to process, and how to come up with output file names. For example, you may keep your templates at a separate directory and output files into your output tree, etc. You may adjust piconf's markup syntax with `piconf_markup_hook`.

	def piconf_dname_hook(level,path,fname=None):
	   return path + (('/'+fname) if fname else '')
	
	def piconf_markup_hook():
	   return [3,ord("="),ord("<"),ord(">")]

	def piconf_fname_hook(level,pattern,path,fname=None,ftype,replace)
	   ofname,cnt = re.subn(pattern,replace,fname,count=1)
	   if ofname and cnt==1 and len(ofname)>0:
	      return path + (('/'..fname) if fname else ''), path + '/' + ofname, piconf_markup_hook()
	   else
	      return nil,nil,nil

##Example

**`$ cat sample.config`**

	a = a if 'a' in locals() else 5
	b = 200
	c = 'Hello, world!'
	if a < 100:
	   d = dict(x=a, z='boo')
	else
	   d = dict(x=10, z='foo')
	
	def square(x)
	   return x*x

**`$ cat sample.template`**

	text\0
	text1,<<<c>>>,text2
	=== if b > 100:
        ===    pass
	text3
        === pass
	text4
	=== for i in range(1,a+1):
        ===    pass
	text5,<<<square(i)>>>,text6,<<<d['z']>>>,text7
	=== pass
	'''text\n8'''

**`$ piconf -v -e 'a=5' -c sample.config sample.template sample`**
	
**`$ cat sample`**

	text\0
	text1,Hello, world!,text2
	text3
	text4
	text5,1,text6,boo,text7
	text5,4,text6,boo,text7
	text5,9,text6,boo,text7
	text5,16,text6,boo,text7
	text5,25,text6,boo,text7
	'''text\n8'''

