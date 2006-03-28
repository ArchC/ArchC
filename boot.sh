#!/bin/sh
test -d config || mkdir config
autoheader
aclocal
libtoolize --force --copy
automake --add-missing --copy
autoconf
