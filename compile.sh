#rm a.out
#g++ --std=c++11 \
#    ftgl/Point.cpp \
#    ftgl/Contour.cpp \
#    ftgl/Vectoriser.cpp \
#    poly2tri/common/shapes.cc \
#    poly2tri/sweep/cdt.cc \
#    poly2tri/sweep/advancing_front.cc \
#    poly2tri/sweep/sweep_context.cc \
#    poly2tri/sweep/sweep.cc \
#    test.cpp \
#    -I/opt/local/include/freetype2 \
#    -I/opt/local/include/freetype2/freetype \
#    -lfreetype \
#    -o a.out
#
#./a.out 'fonts/Lohit-Tamil-Classical.ttf' 'வணக்கம் உலகம்' 48 4 16 'helloworld.obj' 0.25 4

g++ --std=c++11 \
    ftgl/Point.cpp \
    ftgl/Contour.cpp \
    ftgl/Vectoriser.cpp \
    poly2tri/common/shapes.cc \
    poly2tri/sweep/cdt.cc \
    poly2tri/sweep/advancing_front.cc \
    poly2tri/sweep/sweep_context.cc \
    poly2tri/sweep/sweep.cc test.cpp \
    -I/opt/local/include/freetype2 \
    -I/opt/local/include/freetype2/freetype -L/opt/local/lib \
    -lfreetype \
    -o a.out


