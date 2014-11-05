#!/bin/bash

export LD_LIBRARY_PATH="/local/archc/acnightlytester/tools/systemc-2.3.0/lib-linux64/"
cd /local/archc/acnightlytester &> /dev/null
./nightlytester.sh site.conf
rsync -Rrazp -v public_html /home/lsc/projetos/archc/acnightlytester/
cd - &> /dev/null

