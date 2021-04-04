#include <emscripten/emscripten.h>
#include "../Font2OBJ.h"

extern "C" {
    EMSCRIPTEN_KEEPALIVE char* generateMeshJS(std::string fontName, int height, unsigned short bezierSteps, float extrude, float bevelRadius, unsigned short bevelSteps, std::string strUtf8) {
        bevelRadius = bevelRadius * 96 * 0.1;
        printf("Font: %s\n", fontName.c_str());
        printf("Height: %d\n", height);
        printf("Bezier Steps: %d\n", bezierSteps);
        printf("Extrude: %f\n", extrude);
        printf("Bevel Radius: %f\n", bevelRadius);
        printf("Bevel Steps: %d\n", bevelSteps);
        printf("String: %s\n", strUtf8.c_str());

        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
        std::u32string str = conv.from_bytes(strUtf8.c_str());
        Mesh mesh = generateMesh((char*)fontName.c_str(), height, str, bezierSteps, extrude, bevelRadius, bevelSteps);
        mesh.print();
        return (char*)mesh.getOBJData().c_str();
    }
}
