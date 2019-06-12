rm a.out
if [[ "$OSTYPE" == "darwin"* ]]; then
    g++ /System/Library/Frameworks/GLUT.framework/GLUT /System/Library/Frameworks/OpenGL.framework/OpenGL ftgl/Point.cpp ftgl/Contour.cpp ftgl/Vectoriser.cpp poly2tri/common/shapes.cc poly2tri/sweep/cdt.cc poly2tri/sweep/advancing_front.cc poly2tri/sweep/sweep_context.cc poly2tri/sweep/sweep.cc Font2OBJ.cpp -I/usr/local/include/freetype2 -I/usr/local/include/freetype2/freetype -lfreetype -lglut -o a.out
else
    g++ ftgl/Point.cpp ftgl/Contour.cpp ftgl/Vectoriser.cpp poly2tri/common/shapes.cc poly2tri/sweep/cdt.cc poly2tri/sweep/advancing_front.cc poly2tri/sweep/sweep_context.cc poly2tri/sweep/sweep.cc Font2OBJ.cpp -I/usr/include/freetype2/ -I/usr/include/freetype2/freetype -L/opt/local/lib -lfreetype -lglut -lGLU -lGL
fi
./a.out 'hp.ttf' 'HelloWorld' 48 4 16

