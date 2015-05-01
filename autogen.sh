#!/bin/sh

rm -rf autom4te.cache
rm -f aclocal.m4 ltmain.sh config.cache

autoreconf -vif

if [ -z "$NOCONFIGURE" ]; then
  exec ./configure -C "$@"
fi
