#!/bin/bash

cd /local/archc/acnightlytester &> /dev/null
. site.conf
export LD_LIBRARY_PATH="$SYSTEMCPATH/lib-linux64/"
./nightlytester.sh site.conf  $1
rsync -Rrazp -v public_html /home/lsc/projetos/archc/acnightlytester/
cd - &> /dev/null

