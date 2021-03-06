#!/bin/bash
ROOTDIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
cd $ROOTDIR/..

# Cleanups
rm -rf report/
mkdir -p report/

# CPP
lcov -d . -c --initial -o lcov.da
lcov -d . -c -o lcov.da
lcov -r lcov.da "*moc*" -o lcov.da
lcov -r lcov.da "*qrc*" -o lcov.da
lcov -r lcov.da "/usr/include/*" -o lcov.da
lcov -r lcov.da "*3rdparty*" -o lcov.da
lcov -r lcov.da "tst*" -o lcov.da
genhtml -o report --function-coverage -t "microcore" --demangle-cpp lcov.da
mv lcov.da report/lcov.da
