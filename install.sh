#!/bin/bash
trap "rm -f archc.install.tmp" EXIT

#######################################
# This is the ArchC installation script
# Author: Sandro Rigo
# Modified by: Marcus Bartholomeu
#######################################


#######################################
# Find the directory this script is in
#######################################

# Find complete script name
if [ ${0} != ${0#/} ]; then
    #found absolute name
    NAME=$0
else
    #found relative name (remove prefix "./")
    NAME=$PWD/${0#./}
fi

# Find base name
BASENAME=`basename $0`

# Remove basename from complete name
ARCHC_PATH=${NAME%/$BASENAME}



#################################################
# Try to find utilities used to auto-find SystemC
#################################################
HAS_UTILITIES=1

locate dev/null 2>/dev/null 1>&2
if [ $? -ne 0 ] ; then HAS_UTILITIES=0 ; fi

ARCHC_INSTALL_TMP=`printf "line1\nline2\n" | grep "line" | sed -n "2p" 2>/dev/null`
if [ $ARCHC_INSTALL_TMP != line2 ] ; then HAS_UTILITIES=0 ; fi
#################################################


# Geetings
echo 
echo "This is ArchC configuration"
echo "==========================="
echo 
echo "Creating configuration file:"
echo "${ARCHC_PATH}/config/archc.conf"
echo

# Asking for SystemC paths
SYSTEMC_PATH_OK=0
if [ $HAS_UTILITIES -eq 1 ] ; then
    printf "Trying to auto-find SystemC..."
    SYSTEMC_DIRS=`locate "/libsystemc.a" | grep "lib-" | sed "s/\/lib-.*\/libsystemc.a$//" | sort -u`
    if [ \( $? -eq 0 \) -a \( "$SYSTEMC_DIRS" != "" \) ] ; then
	echo " found."
        ls -td $SYSTEMC_DIRS > archc.install.tmp 2>/dev/null
	cat -n archc.install.tmp
	read -e -p "Choose a number or inform the path to SystemC (default: 1) --> " SYSTEMC_PATH; true ${SYSTEMC_PATH:=1}
        case $SYSTEMC_PATH in
            [1-9]*) SYSTEMC_PATH=`cat archc.install.tmp | sed -n "${SYSTEMC_PATH}p"` ;;
        esac
	SYSTEMC_PATH_OK=1
    else
	echo " not found."
    fi
fi

if [ $SYSTEMC_PATH_OK -eq 0 ] ; then
    read -e -p "Inform the path to SystemC (press enter if not installed) --> " SYSTEMC_PATH
fi

if [ -z $SYSTEMC_PATH ] ; then
    echo
    echo "ArchC WARNING: Only compiled simulators work without SystemC."
    echo "ArchC WARNING: Configure a SystemC path if you want to create interpreted simulators."
    echo
else
    
    # Try to find the systemc.h include file
    if [ ! -f "$SYSTEMC_PATH/include/systemc.h" ] ; then
    # TODO: loop until find systemc directory
        echo
        echo "ArchC ERROR: Could not find SystemC include file"
        echo "ArchC ERROR: $SYSTEMC_PATH/include/systemc.h: file not found"
        echo "ArchC ERROR: Please asure that you have a working instalation of SystemC in the indicated directory."
        echo
        exit 1
    fi

    #find the SystemC target arch
    if [ $HAS_UTILITIES -eq 1 ] ; then
        ls -d $SYSTEMC_PATH/lib-* 2>/dev/null | sed "s/.*\/lib-//" > archc.install.tmp 2>/dev/null
        #if it founds more then 1 choice
        if [ 0`cat archc.install.tmp | sed -n "2p"` != 0 ] ; then
            echo "Select a SystemC target:"
            cat -n archc.install.tmp
            read -e -p "Choose a number (default: 1) --> " SYSTEMC_TARGET; true ${SYSTEMC_TARGET:=1}
            SYSTEMC_TARGET=`cat archc.install.tmp | sed -n "${SYSTEMC_TARGET}p" 2>/dev/null`
        else
            SYSTEMC_TARGET=`cat archc.install.tmp`
        fi
    else
        #ask the target arch
        echo
        echo "-->Which TARGET_ARCH you use for your SystemC model's makefiles? (linux)"
        read -e SYSTEMC_TARGET
    fi

    #Default: linux
    true ${SYSTEMC_TARGET:="linux"}

    # Try to find the library file that will be used
    if [ -f "$SYSTEMC_PATH/lib-$SYSTEMC_TARGET/libsystemc.a" ] ; then
        echo "Using library $SYSTEMC_PATH/lib-$SYSTEMC_TARGET/libsystemc.a"
    else
        # TODO: loop until find a valid libsystemc.a
        echo
        echo "ArchC ERROR: Could not find SystemC library file with path"
        echo "ArchC ERROR: $SYSTEMC_PATH/lib-$SYSTEMC_TARGET/libsystemc.c"
        echo "ArchC ERROR: Please asure that you have a working instalation of SystemC in the indicated directory."
        echo
        exit 1
    fi
fi
rm -f archc.install.tmp


# Asking for Compiler path
echo
echo "-->Which compiler do you want to use with ArchC? (g++)"
read -e -p "-->" CC; true ${CC:="g++"}


#Asking for compiler flags
echo
echo "--> Inform the flags that you want to pass to the compiler:"
echo "    Note: You can always change these flags by editing the makefile"
echo "          (Makefile.archc) that is generated for your models."
echo

read -e -p "    --> Optimization (none): " optflag
read -e -p "    --> Debug          (-g): " debugflag
read -e -p "    --> Other       (-Wall -Wno-deprecated -Wno-char-subscripts): " otherflag
echo

#Defaults:
true ${optflag:=""}  #do not do optimizations
true ${debugflag:="-g"}  #insert debug info
true ${otherflag:="-Wall -Wno-deprecated -Wno-char-subscripts"}  #all warnings

#Erase flags if the user has typed "none"
optflag=${optflag%"none"}
debugflag=${debugflag%"none"}
otherflag=${otherflag%"none"}



#Writing configuration file

[ -d ${ARCHC_PATH}/config ] || mkdir ${ARCHC_PATH}/config
[ -L ${ARCHC_PATH}/systemc ] || ln -s ${SYSTEMC_PATH} ${ARCHC_PATH}/systemc

cat > ${ARCHC_PATH}/config/archc.conf <<EOF
# ArchC Configuration File 
# This file is automatically generated by the ArchC Installation script

#---------------------------------------------------------------------------
# Paths
#---------------------------------------------------------------------------
ARCHC_PATH = ${ARCHC_PATH}
SYSTEMC_PATH = ${SYSTEMC_PATH}
CC = ${CC}

#---------------------------------------------------------------------------
# Compiler Flags
#---------------------------------------------------------------------------
OPT = ${optflag}
DEBUG = ${debugflag}
OTHER = ${otherflag}

#---------------------------------------------------------------------------
# Simulator Flags
#---------------------------------------------------------------------------
TARGET_ARCH = ${SYSTEMC_TARGET}
EOF

if [ $? -ne 0 ] ; then echo ArchC: Error writing configuration file \"${ARCHC_PATH}/config/archc.conf\"; exit 1; fi


cat <<EOF

-----------------------------------------------------------------------
Attention! You MUST set an environment variable called ARCHC_PATH,
           containing the path where you installed the ArchC package,
           otherwise ArchC tools will not run properly.
           Example command for sh-compatible shell:
              ARCHC_PATH=${ARCHC_PATH}
-----------------------------------------------------------------------

ArchC: Configuration Finished Successfully

Thanks for using ArchC.

You will find up to date information, models,
documents and more at http://www.archc.org.

If you have doubts, suggestions or want more information that you
could not find on the website, you can reach us at archc@lsc.ic.unicamp.br.

We hope you enjoy using ArchC!

Best regards,
               The ArchC Team
 
EOF
