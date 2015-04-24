# Font23D
Font23D is a C++ library for creating a 3d mesh of any Text in the given True type font.

Steps:
1. The Library uses FreeType Library to decode the fonts and converts the Text into Vector.
2. Then the vector data is converted into triangles using Ploy2Tri library.
3. This gives us the triangles of the complete text in a plane.
4. The plane is then extruded into 3d object and then creates a closed mesh.
5. The Mesh is then rendered using OpenGL.

Plans for Future:
1. Allow Option to export the Mesh into OBJ file.
2. Add Support for Unicode text.