#!/bin/sh
aclocal
autoconf
test -d config || mkdir config
automake --add-missing
