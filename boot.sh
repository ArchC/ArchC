#!/bin/sh
test -d config || mkdir config
aclocal
libtoolize --force --copy
automake --add-missing --copy
autoconf
