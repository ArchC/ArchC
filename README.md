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

Build
------------
ArchC package uses the GNU autotools framework to build the source
files and install them into the host system. .

1.
```bash
./boot.sh
```
2.
```bash
./configure --with-systemc=<systemc-path>
```
Optional flags:
  * --with-binutils=<binutils-path> -> if you plan to generate binary utilities
  * --with-gdb=<gdb-path> -> if you plan to generate debugger (gdb)
  * --prefix=<install-dir> -> By default, the installation process will install the package in the /usr/local directory. If you want any other directory to be used, just use the --prefix flag.

3.
```bash
make
```

Installation
------------

```bash
make install
```

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
