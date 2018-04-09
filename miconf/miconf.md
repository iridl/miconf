#miconf
##Name
`miconf` - lightweight configuration utility with Lua 
##Synopsis
	miconf -h
	miconf [options] -r directory
	miconf [options] template_file output_file
##Description
`miconf` is a lightweight configuration utility based on simple template substitution technique and [Lua](http://www.lua.org/) expressions. It executes a configuration file (a Lua program), and then uses the results of the execution (its global state) in template substitution. It reads `template_file` and substitutes `<<<`*lua expression*`>>>` placeholders with evaluated *lua expression*. It also executes lines that start with `===`. 

For example:

	. . .
	=== for x = 1, 3 do
	<<<x>>> ^ 3 = <<< x^3 >>>
	=== end
	. . .

will result in the following output:

	. . .
	1 ^ 3 = 1
	2 ^ 3 = 8
	3 ^ 3 = 27
	. . .

		
When `miconf` is run in recoursive `-r` mode, it traverses `directory` recoursively and processes files based on the pattern (`-p` option) and the output of callback functions.

`miconf` executable is a self sufficient executable that requires only a C runtime.
      
##Options

| Option 	| Default value 	| Description 	| Example 	| 
| :--- 	| :--- 	| :--- 	| :--- 	|
| `-c file` 	| `'config.lua'` 	| config file 	| `-c system.config` 	|
| `-e block` 	| `''` 	| config block 	| `-e 'host="foo"; ip="127.0.0.1"'`	|
| `-p pattern` 	| `'[.]template$'` 	| template file name pattern (see `miconf_fname_hook`) 	| `-p '[.]miconf([.]?)'` 	|
| `-s replace` 	| `''` 	| file name pattern replacement (see `miconf_fname_hook`) 	| `-s '%1'` 	|
| `-t` 	| 	| preserve temp files 	| 	|
| `-m` 	| 	| disable chmod 	| 	|
| `-v` 	| 	| verbose 	| 	|

##Default hooks

The following "hooks" are used if you do not redefine them in your configuration files. 
Function `miconf_markup_hook` is used in both normal abd recursive modes. Functions `miconf_dname_hook` and `miconf_fname_hook` get called in recoursive mode only. Function `miconf_dname_hook` is invoked for each subdirectory, and `miconf_fname_hook` is called for each regular file. `miconf_dname_hook` has to return a full path to the subdirectory to traverse, or `nil` to ignore the whole subdirectory. `miconf_fname_hook` has to return input file path, output file path and markup syntax description table (see `miconf_markup_hook` below), or `(nil,nil,nil)` if you want to ignore the file. These functions allow for customization of miconf's behavior, i.e. what parts of your tree and what files to process, and how to come up with output file names. For example, you may keep your templates at a separate directory and output files into your output tree, etc. You may adjust miconf's markup syntax with `miconf_markup_hook`. Currently the first element of the array returned by `miconf_markup_hook` is not used.

	function miconf_dname_hook(level,path,file)
	   return path..(file and ("/"..file) or "")
	end
	
	function miconf_markup_hook()
	   return {3,string.byte("="),string.byte("<"),string.byte(">")}
	end

	function miconf_fname_hook(level,pattern,path,file,type,replace)
	   ofile,cnt = file:gsub(pattern,replace,1)
	   if ofile and cnt==1 and ofile:len()>0 then
	      return path..(file and ("/"..file) or ""), path.."/"..ofile, miconf_markup_hook()
	   else
	      return nil,nil,nil
	   end
	end


##Example

**`$ cat sample.config`**

	a = a or 5
	b = 200
	c = "Hello, world!"
	if a < 100 then
	   d = {x=a, "boo"}
	else
	   d = {x=10, "foo"}
	end
	
	function square(x)
	   return x*x
	end


**`$ cat sample.template`**

	text0
	text1,<<<c>>>,text2
	=== if b > 100 then
	text3
	=== end
	text4
	=== for i = 1, a do
	text5,<<<square(i)>>>,text6,<<<d[1]>>>,text7
	=== end
	text8

**`$ miconf -v -e 'a=5' -c sample.config sample.template sample`**
	
**`$ cat sample`**

	text0
	text1,Hello, world!,text2
	text3
	text4
	text5,1,text6,boo,text7
	text5,4,text6,boo,text7
	text5,9,text6,boo,text7
	text5,16,text6,boo,text7
	text5,25,text6,boo,text7
	text8

