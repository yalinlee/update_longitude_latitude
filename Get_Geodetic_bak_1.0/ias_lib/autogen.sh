#! /bin/sh

# Run this to generate all the auto-generated files needed by the GNU
# configure program

libtoolize --automake
aclocal
autoheader --force
automake --add-missing
autoconf

# If we have QT (the qmake command is known), then
# run autogen on the IAS QT library, which is a separate package
which qmake >& /dev/null && ( cd ias_qt_lib && ./autogen.sh )

exit 0
