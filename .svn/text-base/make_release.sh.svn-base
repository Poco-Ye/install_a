#!/bin/bash
# used to generate the files that will be released to customer.

NOT_RELEASE_FILE=(
	"src/rtk_onlineupgrade.cpp"
	"src/rtk_programmer.cpp"
	"src/rtk_verify.cpp"
	"src/rtk_wget.cpp"
	"src/rtk_xml.cpp"
	"include/rtk_onlineupgrade.h"
	"include/rtk_verify.h"
	"include/rtk_wget.h"
	"include/rtk_xml.h"
	"include/svnver.h"
)

if [ "$1" = "" ]; then
	echo "Usage:"
	echo "$0 dir_path"	
else
	REV=$(svnversion -c | awk -F":" '{ print $2 }')
	echo "REV=$REV"

	echo "rm -rf $1  &&  mkdir -p $1"
	rm -rf $1
	mkdir -p $1

	echo "copy files"
	cp -rf include/ lib/ src/ $1
	cp -rf install_a.cpp Makefile Makefile.pc $1

	cd $1
	# modify Makefile...
	sed -i -e /SUBDIRS/d Makefile
	sed -i -e s/REV=.*/REV=$REV/ Makefile
	# remove those files that do not release...
	find -name '.svn' | xargs rm -rf
	find -name '*.o' |xargs rm -rf
	for file in ${NOT_RELEASE_FILE[*]}
	do
		rm -rf $file
	done
	
	# tar the released source code..
	echo "tar released source code to install.tar.bz2"
	tar cjf ../install.tar.bz2 *
	cd -	
fi
