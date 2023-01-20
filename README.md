# Alpha
Complete Compiler and Virtual Machine for Alpha

Made in 2020 with [Labis Anargyrou](https://github.com/LabisAnargyrou) for [Languages & Compilers (hy340)](https://www.csd.uoc.gr/~hy340/) at [CSD](https://www.csd.uoc.gr/)

## Linux Installation
The Alpha compiler requires GNU Bison & Flex:
```
sudo apt install bison
sudo apt install flex
```
Download & build Alpha:
```
git clone "https://github.com/PhivPap/AlphaLang_hy340.git"
cd AlphaLang_hy340
make
```

## Usage
Compile an Alpha source file with:
```
./alphac <source.a> -o <bytecode.abc>
```
Execute an Alpha binary file with:
```
./alpha <bytecode.abc>
```

Alternatively, do both in one line:
```
./alphac <source.a> -run
```

## In other news
While the language is sadly not described in this repo, you can try guessing with the provided testfiles. You can, however, rest assured knowing:
* The Alpha compiler has:
  * dead code elimination
  * temporary variable reuse optimization
* The Alpha virtual machine has:
  * the following library functions: 
    * ``print(...)`` - Variadic print function. It catches direct or indirect object self reference loops
    * ``input()`` - Returns a line from console. Auto converts to other-than-string possible types.
    * ``objectmemberkeys(t)`` - Returns a table with all the keys of "t" indexed from 0 and up.
    * ``objecttotalmembers(t)`` - Returns the number of entries in "t".
    * ``objectcopy(t)`` - Returns a shalow copy of "t".
    * ``totalarguments()`` - Returns the number of arguments passed to the function from where it was called.
    * ``argument(i)`` - Returns the ith argument of the function from where this was called in. Can be used with totalarguments() to create an Alpha variadic function.
    * ``typeof(x)`` - Returns the type of "x" as a string.
    * ``strtonum(s)`` - Converts string "s" to num.
    * ``sqrt(n)`` - You won't believe what this one does.
    * ``cos(n)`` -
    * ``sin(n)`` -
  * table functors (double dot ".." operator)
  * tables which are implemented as expanding open addressing linearly probed hash tables
  * tables which receive any type as key and value
  * string concat (ikr **damn**)
  * perfectly hashed library functions (i.e. they are called in O(1))
