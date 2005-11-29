#!/bin/sh

BINUTILS_TREE="binutils 
binutils/opcodes
binutils/gas
binutils/gas/config
binutils/bfd
binutils/include
binutils/include/opcode
binutils/include/elf"

FILES_TO_PATCH="bfd/archures.c
bfd/Makefile.in
bfd/bfd-in2.h
bfd/config.bfd
bfd/configure
bfd/targets.c
config.sub
gas/configure.tgt
gas/Makefile.in
opcodes/configure
opcodes/Makefile.in"

#
# command line parsing
#
me=`echo "$0" | sed -e 's,.*/,,'`

usage="\
Usage: $0 [options] <archc source file> <architecture name>

Create gas dependent source files and copy them to the binutils tree.

Options:
  -c, --create-only  only create the files, do not copy to binutils tree
  -h, --help         print this help
  -v, --version      print version number

Report bugs and patches to ArchC Team."

version="\
ArchC assembler generator script -beta version-

2005 - Only for internal use"

help="
Try \`$me --help' for more information."

CREATE_ONLY="0"

# Parse command line
while test $# -gt 0 ; do
  case $1 in
    --version | -v )
       echo "$version" ; exit 0 ;;
    --help | --h* | -h )
       echo "$usage"; exit 0 ;;
    --create-only | -c )
       CREATE_ONLY="1"; shift; break ;;  
#    -- )     # Stop option processing
#       shift; break ;;
    -* )
       echo "$me: invalid option $1$help"
       exit 1 ;;
    * )
       break ;;
  esac
done

case $# in
 0 | 1) echo "$me: missing argument$help" >&2
    exit 1;;
 2 | 3) ;;
 *) echo "$me: too many arguments$help" >&2
    exit 1;;
esac

#
# check for environment variable definitions
#
if [ -z "$ARCHC_PATH" ]; then
  echo "ARCHC_PATH environment variable not set"
  exit 1
fi

if [ -z "$BINUTILS_PATH" ]; then
  echo "BINUTILS_PATH environment variable not set"
  exit 1
fi

# check for '/' at the end and take it out
ARCHC_DIR=`echo "$ARCHC_PATH" | sed -e 's/\/$//'`
BINUTILS_DIR=`echo "$BINUTILS_PATH" | sed -e 's/\/$//'`

#ACASM_DIR="$ARCHC_DIR"/src/acasm
SEDFILES_DIR="$ARCHC_DIR"/include/acasm

FILES_TO_COPY="gas/config/tc-$2.c
gas/config/tc-$2.h
opcodes/$2-opc.c
include/opcode/$2.h
include/elf/$2.h
bfd/elf32-$2.c
bfd/cpu-$2.c"

# creates binutils directory tree (if none was built)
for directory in $BINUTILS_TREE
do
  if [ ! -d "$directory" ]; then
     echo "Creating directory $directory";
     mkdir $directory > /dev/null 2>&1
     if [ $? -ne 0 ]; then
	echo "Cannot create directory $directory."
	exit 1
     fi
  fi
done

# executes acasm to generate the files in the binutils tree
echo "Executing acasm..."
$ARCHC_DIR/bin/acasm $1 -a$2
[ $? -ne 0 ] && exit $?

# copies the tc-templ.c to local directory
cp -f $ARCHC_DIR/include/acasm/tc-templ.c binutils/gas/config/
[ $? -ne 0 ] && exit $?

#generates the final tc-xxxxx file
sed -f binutils/gas/config/tc-templ.sed binutils/gas/config/tc-templ.c > binutils/gas/config/tc-templ.ct
sed s/xxxxx/$2/g binutils/gas/config/tc-templ.ct > binutils/gas/config/tc-templ.cr
cat binutils/gas/config/tc-templ.cr binutils/gas/config/tc-funcs.c > binutils/gas/config/tc-$2.c
[ $? -ne 0 ] && exit $?

if [ "$CREATE_ONLY" -ne "0" ]; then
  echo "Done. No files copied."
  exit 0
fi

# check if we need to patch the files
$BINUTILS_DIR/config.sub $2-elf > /dev/null 2>&1

if [ $? -ne 0 ]; then
# applies the patch 
    echo "Patching... "
    for file in $FILES_TO_PATCH
    do
      
      if [ ! -f "$BINUTILS_DIR/$file" ]; then

	  if [ $file = "gas/configure.tgt" ]; then
	# Starting from 2.16, binutils uses configure.tgt
	# For compatibility with older versions, redirect to 'configure'
	      file="gas/configure"
	      cp -f $SEDFILES_DIR/binutils/gas/configure.tgt.sed $SEDFILES_DIR/binutils/gas/configure.sed > /dev/null 2>&1
	  else           	  
  	    echo "Source file $file not found."
  	    exit 1
	  fi
      fi

  # creates a backup
      mv -f $BINUTILS_DIR/$file $BINUTILS_DIR/$file.bkp > /dev/null 2>&1
      if [ $? -ne 0 ]; then
	  echo "Cannot move file $file."
	  exit 1
      fi

      sed s/xxxxx/$2/g $SEDFILES_DIR/binutils/$file.sed > binutils/$file.sed
      sed -f binutils/$file.sed $BINUTILS_DIR/$file.bkp > $BINUTILS_DIR/$file

    done
   
    chmod a+x $BINUTILS_DIR/config.sub
    chmod a+x $BINUTILS_DIR/bfd/config.bfd

else
    echo "Configuration triplet $2-elf found. Configuration files won't be patched."
fi

# copies the generated files into binutils tree
echo "Copying files to binutils source tree..."
for file in $FILES_TO_COPY
do
  cp -f binutils/$file $BINUTILS_DIR/$file
  [ $? -ne 0 ] && exit $?
done

echo "All done successfully."
