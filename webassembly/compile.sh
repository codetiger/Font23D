#!/bin/bash

if [[ ! -d build ]]; then
    set -e
    cd "$(dirname "$0")/"

    # See https://download.savannah.gnu.org/releases/freetype/
    FT_VERSION=2.10.4

    mkdir -p freetype-tmp
    pushd freetype-tmp >/dev/null

    if [[ ! -d freetype-$FT_VERSION ]]; then
        TARFILE=freetype-$FT_VERSION.tar.gz
        if [[ ! -f "$TARFILE" ]]; then
            URL=https://download.savannah.gnu.org/releases/freetype/$TARFILE
            echo "Fetching $URL"
            curl -LO "$URL"
        fi
        echo "Extracting $TARFILE"
        tar xzf $TARFILE
    fi

    popd >/dev/null

    FTSRC=freetype-tmp/freetype-$FT_VERSION
    rm -rf freetype
    mv -f "$FTSRC" freetype

    cd freetype
    mkdir build
    cd build
    emcmake cmake ..
    emmake make
    emmake make install
fi

if [[ ! -d fonts ]]; then
    mkdir fonts
    cp ../fonts/* fonts
fi

em++ \
    ../ftgl/Point.cpp \
    ../ftgl/Contour.cpp \
    ../ftgl/Vectoriser.cpp \
    ../poly2tri/common/shapes.cc \
    ../poly2tri/sweep/cdt.cc \
    ../poly2tri/sweep/advancing_front.cc \
    ../poly2tri/sweep/sweep_context.cc \
    ../poly2tri/sweep/sweep.cc \
    web.cc \
    -I/usr/local/include/freetype2 \
    -I/usr/local/include/freetype2/freetype \
    -L/usr/local/lib/ \
    -lfreetype \
    --preload-file fonts/coolveticarg.ttf \
    --preload-file fonts/hp.ttf \
    --preload-file fonts/Hursheys.ttf \
    --preload-file fonts/Lohit-Tamil-Classical.ttf \
    -o web.js \
    -s EXTRA_EXPORTED_RUNTIME_METHODS='["cwrap"]'

cp index.html /usr/local/var/www/
cp index.css /usr/local/var/www/
cp web.js /usr/local/var/www/
cp web.wasm /usr/local/var/www/
cp web.data /usr/local/var/www/