#!/bin/bash

LIB_DIR=$(dirname $(realpath $0))
SRC_DIR=$LIB_DIR/../../src/data

rm $SRC_DIR/lib.*

C=1

if [ 1 -eq $C ]; then
  COMP="gzip -c"
  OUT=.gz
else
  COMP="cat"
  OUT=
fi

#cat $LIB_DIR/jquery.min.js \
#    $LIB_DIR/knockout.min.js \
#    $LIB_DIR/knockout.mapping.min.js \
#    $LIB_DIR/sammy.min.js \
#    $LIB_DIR/bootstrap.min.js \
#    | $COMP > $SRC_DIR/lib.js$OUT

#cat $LIB_DIR/bootstrap.min.css \
#    | $COMP > $SRC_DIR/lib.css$OUT

cat $LIB_DIR/jquery.slim.min.js \
    $LIB_DIR/knockout.min.js \
    $LIB_DIR/knockout.mapping.min.js \
    | $COMP > $SRC_DIR/lib.js$OUT
