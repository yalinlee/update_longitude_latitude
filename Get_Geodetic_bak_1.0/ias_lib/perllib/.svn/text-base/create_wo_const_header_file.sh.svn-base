#!/bin/sh
echo "/* ************ */" > $2
echo "/* READ ME ---- READ ME ---- READ ME */" >> $2
echo "/* This file was auto generated from the $1 perl module. */" >> $2
echo "/* ************ */" >> $2
echo "" >> $2
echo "#ifndef IAS_WO_CONST_H" >> $2
echo "#define IAS_WO_CONST_H" >> $2
echo "" >> $2
sed -n '
s|^#*$||
s|^use constant\s*||
s|\(\w*\)\s*=>\s*\"\(.*\)\".*|const QString \1\(\"\2\"\);| p
s|\(\w*\)\s*=>\s*\([0-9]*\).*|const int \1\(\2\);| p
s|^#\s*\(.*\)\s*$|// \1| p
s|^\(\s*\)#\s*\(.*\)\s*$|\1// \2| p
' < $1 >> $2
echo "" >> $2
echo "#endif /* IAS_WO_CONST_H */" >> $2
