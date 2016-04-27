ArchC
=====

**Architecture Description Language**

ArchC is a powerful and modern open-source architecture description language designed at University of Campinas by the ArchC team in the Computer Systems Laboratory, Institute of Computing.

License
-------
 - ArchC tools are provided under the GNU GPL license.
   See [Copying](COPYING) file for details on this license.

 - ArchC utility library, i.e. all files stored in the src/aclib
   directory of the ArchC source tree, are provided under the GNU LGPL
   license. See the [Copying Lib](COPYING.LIB) file for details on this license.


Requirements
------------

1. GNU autotools (m4 autoconf automake libtool)

2. SystemC  
   http://www.accellera.org/downloads/standards/systemc  
   https://github.com/ArchC/SystemC.git

3. elfutils (libelf and libdw) - Optional for High Level Trace feature  
   https://fedorahosted.org/elfutils/  
   git://git.fedorahosted.org/git/elfutils.git

Build
------------
ArchC package uses the GNU autotools framework to build the source
files and install them into the host system.

1.
```bash
./autogen.sh
```
2.
```bash
./configure
```
Optional flags:
  * --prefix=*\<install-dir\>* -> if you want install ArchC in any other directory. The default is /usr/local
  * --with-systemc=*\<systemc-install-path\>*    -> if you plan to generate 'acsim' tool
  * --with-binutils=*\<binutils-src-path\>* -> if you plan to generate binary utilities (acasm)
  * --with-gdb=*\<gdb-src-path\>* -> if you plan to generate debugger (gdb)

3.
```bash
make
```

Installation
------------

```bash
make install
```

Environment
------------

```bash
. env.sh
``` 

The above command will set the ArchC Environment in the current terminal.
Add it in ~/.bashrc to set the ArchC environment automatically

Contributing
------------

See [Contributing](CONTRIBUTING.md)


More
----

Remember that ArchC models and SystemC library must be compiled with
the same GCC version, otherwise you will get compilation problems.

Several documents which further information can be found in the 'doc'
subdirectory.

You can find language overview, models, and documentation at
http://www.archc.org



Thanks for the interest. We hope you enjoy using ArchC!

The ArchC Team
Computer Systems Laboratory (LSC)
IC-UNICAMP
http://www.lsc.ic.unicamp.br
