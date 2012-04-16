#!/bin/sh
test -d config || mkdir config
autoheader
aclocal
glibtoolize --version &> /dev/null
if [ $? -eq 0 ]; then
    glibtoolize --force --copy
else
    libtoolize --force --copy
fi
automake --add-missing --copy
autoconf
