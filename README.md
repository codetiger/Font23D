# Font23D
Font23D is a C++ library for creating a 3d mesh of any Text in the given True type font.

Steps:
* The Library uses FreeType Library to decode the fonts and converts the Text into Vector.
* Then the vector data is converted into triangles using Ploy2Tri library.
* This gives us the triangles of the complete text in a plane.
* The plane is then extruded into 3d object and then creates a closed mesh.
* The Mesh is then rendered using OpenGL.

Plans for Future:
* Allow Option to export the Mesh into OBJ file.
* Add Support for Unicode text.