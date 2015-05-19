rm a.out
g++ ftgl/Point.cpp ftgl/Contour.cpp ftgl/Vectoriser.cpp poly2tri/common/shapes.cc poly2tri/sweep/cdt.cc poly2tri/sweep/advancing_front.cc poly2tri/sweep/sweep_context.cc poly2tri/sweep/sweep.cc Font2OBJ.cpp -I/usr/include/freetype2/ -lfreetype -lglut -lGLU -lGL
./a.out 'hp.ttf' 'Gg Iyan 3d' 48 2 4