#!/bin/sh

# This scripts creates a tar.gz file with all the linux installation.
# Also it creates a RPM package.
# @param The version of the project

# To create the source tar file you have to install next packages:
# autoconf, automake, libtool

# To create RPM in Fedora you have to follow this section on the link:
#   https://fedoraproject.org/wiki/How_to_create_an_RPM_package#Preparing_your_system

# To create RPM in CentOs you have to follow this link:
#   http://wiki.centos.org/HowTos/SetupRpmBuildEnvironment

project="rpcdds"

function installer
{
    # Copy licenses.
    cp ../../../../doc/licencias/RPCDDS_LICENSE.txt tmp/$project
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    cp ../../../../doc/licencias/LGPLv3_LICENSE.txt tmp/$project
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi

    # Copy documentation.
    mkdir -p tmp/$project/doc
    mkdir -p tmp/$project/doc/pdf
    cp "../../../../doc/RPCDDS - Installation Manual.pdf" tmp/$project/doc/pdf
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    cp "../../../../doc/RPCDDS - User Manual.pdf" tmp/$project/doc/pdf
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    cp -r "../../../../output/doxygen/html" tmp/$project/doc
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    cp "../../../../output/doxygen/latex/refman.pdf" "tmp/$project/doc/pdf/RPCDDS - API C++ Manual.pdf"
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi

    # Copy README
    cp ../../../../README.html tmp/$project/doc
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi

    # Copy example.
    mkdir -p tmp/$project/examples/C++
    cp -r ../../../../examples/C++/DDS/* tmp/$project/examples/C++
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi

    # RPCDDS headers
    mkdir -p tmp/$project/include
    cp -r ../../../../includetmp/rpcdds tmp/$project/include
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi

    # Copy eProsima header files
    mkdir -p tmp/$project/include/rpcdds/eProsima_cpp
    cp $EPROSIMADIR/code/eProsima_cpp/eProsima_auto_link.h tmp/$project/include/rpcdds/eProsima_cpp
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    cp $EPROSIMADIR/code/eProsima_cpp/eProsimaMacros.h tmp/$project/include/rpcdds/eProsima_cpp
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi

    # Copy threadpool header files.
    mkdir -p tmp/$project/extra
    cp -r $EPROSIMA_LIBRARY_PATH/threadpool-0_2_5-src/threadpool tmp/$project/extra
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi

    # Copy RPCDDS sources
    cd ../../../..
    cp --parents `cat building/makefiles/common_sources` utils/installers/rti/linux/tmp/$project
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    cp --parents `cat building/makefiles/rpcdds_sources` utils/installers/rti/linux/tmp/$project
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    cd utils/installers/rti/linux
    find tmp/$project/src -type f -exec sed -i -e 's/#include "fastrpc/#include "rpcdds/' {} \;

    # Copy autoconf configuration files.
    cp configure_rpcdds.ac tmp/$project/configure.ac
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    cp Makefile_rpcdds.am tmp/$project/Makefile.am
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    cp include_Makefile.am tmp/$project/include/Makefile.am
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi

    # Generate autoconf files
    cd tmp/$project
    sed -i "s/VERSION/${version}/g" configure.ac
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    sed -i "s/VERSION_MAJOR/`echo ${version} | cut -d. -f1`/g" Makefile.am
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    sed -i "s/VERSION_MINOR/`echo ${version} | cut -d. -f2`/g" Makefile.am
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    sed -i "s/VERSION_RELEASE/`echo ${version} | cut -d. -f3`/g" Makefile.am
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    sourcefiles=$(cat ../../../../../../building/makefiles/common_sources | sed -e ':a;N;$!ba;s/\n/ /g')
    sourcefiles+=" "
    sourcefiles+=$(cat ../../../../../../building/makefiles/rpcdds_sources | sed -e ':a;N;$!ba;s/\n/ /g')
    sed -i -e "s#RPCDDS_SOURCES#$sourcefiles#" Makefile.am
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    includefiles=$(cat ../../../../../../building/includes/rpcdds_includes | sed -e 's#^#rpcdds/#')
    includefiles=$(echo $includefiles | sed -e ':a;N;$!ba;s/\n/ /g')
    includefiles+=" rpcdds/eProsima_cpp/eProsima_auto_link.h rpcdds/eProsima_cpp/eProsimaMacros.h"
    sed -i -e "s#INCLUDE_FILES#$includefiles#" include/Makefile.am
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    autoreconf --force --install
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    cd ../..

    # Copy Java classes.
    mkdir -p tmp/$project/classes
    cp ../../../../classes/rpcddsgen.jar tmp/$project/classes
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi

    # Copy Java dependent classes.
    cp ../../../../classes/antlr-2.7.7.jar tmp/$project/classes
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    cp ../../../../classes/antxr.jar tmp/$project/classes
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    cp ../../../../classes/stringtemplate-3.2.1.jar tmp/$project/classes
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi

    # Copy script
    mkdir -p tmp/$project/scripts
    cp ../../../../scripts/rpcddsgen.sh tmp/$project/scripts
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi
    chmod 755 tmp/$project/scripts/rpcddsgen.sh

    # Copy MessageHeader.idl
    mkdir -p tmp/$project/idl
    cp ../../../../idl/MessageHeader.idl tmp/$project/idl
    errorstatus=$?
    if [ $errorstatus != 0 ]; then return; fi

    # Erase backup files from vim
    find tmp/ -iname "*~" -exec rm -f {} \;

    cd tmp
    tar cvzf "../${project}_${version}-RTIDDS_5.0.0.tar.gz" $project
    errorstatus=$?
    cd ..
    if [ $errorstatus != 0 ]; then return; fi
}

if [ $# -lt 1 ]; then
    echo "Needs as parameter the version of the product $project"
    exit -1
fi

version=$1

# Get distro version
. $EPROSIMADIR/scripts/common_pack_functions.sh getDistroVersion

# Create the temporaly directory.
mkdir tmp
mkdir tmp/$project

installer

# Remove temporaly directory
rm -rf tmp

if [ $errorstatus == 0 ]; then
    echo "INSTALLER SUCCESSFULLY"
else
    echo "INSTALLER FAILED"
fi

exit $errorstatus
