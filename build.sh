#!/bin/bash

PRKDIR=`cd $(dirname "$0") && pwd`
BUILDDIR="$PRKDIR/build"
BINDIR=$PRKDIR/../build/bin
UMMAPIO=$PRKDIR/../ummap-io/build
MAKEDEFS_PATH="$PRKDIR/common/make.defs"
MAKEDEFS_CHECK=`ls $PRKDIR/common/make.defs 2>/dev/null`
CRAY_CHECK=`cc -V 2>&1 | grep Cray`

if [[ $1 == "make" ]]
then
    # export CC=$2
    # export MPICC=$2
    # export CFLAGS="-I$UMMAPIO/include"
    # export LDFLAGS="-L$UMMAPIO/lib"
    # export LIBS="-lummapio"

    # if [[ $CRAY_CHECK == "" ]]
    # then
    #     export LIBS="$LIBS -pthread -lrt"
    # fi

    if [[ $MAKEDEFS_CHECK == "" ]]
    then
        if [[ $CRAY_CHECK == "" ]]
        then
            cp $MAKEDEFS_PATH.gcc  $MAKEDEFS_PATH
        else
            cp $MAKEDEFS_PATH.cray $MAKEDEFS_PATH
        fi
    fi

    cd $PRKDIR

    make allmpi1_ummap &&
    cp $PRKDIR/MPI1_ummap/Synch_p2p/p2p $BINDIR/prk_p2p.out &&
    cp $PRKDIR/MPI1_ummap/Transpose/transpose $BINDIR/prk_transpose.out &&
    cp $PRKDIR/MPI1_ummap/Stencil/stencil $BINDIR/prk_stencil.out &&
    cp $PRKDIR/MPI1_ummap/Reduce/reduce $BINDIR/prk_reduce.out &&
    cp $PRKDIR/MPI1_ummap/PIC-static/pic $BINDIR/prk_pic.out
else
    cd $PRKDIR

    rm $BINDIR/prk_* 2>/dev/null

    if [[ $MAKEDEFS_CHECK != "" ]]
    then
        make clean
    fi
fi

