# Alpha
Complete Compiler and Virtual Machine for Alpha

Made in 2020 with [Labis Anargyrou](https://github.com/LabisAnargyrou) for [Languages & Compilers (hy340)](https://www.csd.uoc.gr/~hy340/) at [CSD](https://www.csd.uoc.gr/)

<!-- ![Alpha](https://user-images.githubusercontent.com/74933714/236004194-0698ca98-b8e2-435c-bdbf-d165bf225023.png) -->

![alpha2](https://user-images.githubusercontent.com/74933714/236016864-b789c9ab-7b33-49e5-afb1-e68665360a96.png)

![alpha3](https://user-images.githubusercontent.com/74933714/236016890-c14e4593-df54-4220-831b-5cdc23ca55e2.png)


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
./alphac <source.a> --run
```

Print the quads and human readable byte code with:
```
./alphac <source.a> --quads --instructions
```

## In other news
While the language is sadly not described in this repo, you can try guessing with the provided testfiles. You can, however, know the following:
* The Alpha compiler has:
  * compile-time expression evaluation
  * dead code elimination
  * temporary variable reuse optimization
  * minimal evaluation
* The Alpha virtual machine has:
  * the following library functions: 
    * ``print(...)`` - Variadic print function. Catches direct or indirect object self reference loops
    * ``input()`` - Returns line from console. Implicit convertion to other-than-string possible types.
    * ``objectmemberkeys(t)`` - Returns a table with the keys of *t* indexed from 0 and up.
    * ``objecttotalmembers(t)`` - Returns the number of entries in *t*.
    * ``objectcopy(t)`` - Returns a shalow copy of *t*.
    * ``totalarguments()`` - Returns argc of the calling function.
    * ``argument(i)`` - Returns the *i*th argument of the function from where this was called in. Can be used with totalarguments() to create an Alpha variadic function.
    * ``typeof(x)`` - Returns the type of *x* as a string.
    * ``strtonum(s)`` - Converts string *s* to num.
    * ``sqrt(n)`` - 
    * ``cos(n)`` -
    * ``sin(n)`` -
  * table functors (double dot ".." operator)
  * tables which are implemented as expanding open addressing linearly probed hash tables
  * tables which receive any type as key and value
  * string concat (ikr **damn**)
  * perfectly hashed library functions (i.e. they are called in O(1))
  * complete garbage collection
