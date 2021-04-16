#include "Font2OBJ.h"

#include <string>

int main(int argc, char **argv) {
	char* fontName = argv[1];
	printf("Font: %s\n", fontName);
	int height = atoi(argv[3]);
	printf("Height: %d\n", height);
	unsigned short bezierSteps = atoi(argv[4]);
	printf("Bezier Steps: %d\n", bezierSteps);
	float extrude = atof(argv[5]);
	printf("Extrude: %f\n", extrude);
	std::string fileName(argv[8]);
	printf("Filename: %s\n", fileName.c_str());
    float bevelRadius = atof(argv[6]) * 96 * 0.1;
    printf("Bevel Radius: %f\n", bevelRadius);
    unsigned short bevelSteps = atoi(argv[7]);
    printf("Bevel Steps: %d\n", bevelSteps);

	std::string strUtf8 = argv[2];
	printf("String: %s\n", strUtf8.c_str());
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    std::u32string str = conv.from_bytes(strUtf8);
	Mesh mesh = generateMesh(fontName, height, str, bezierSteps, extrude, bevelRadius, bevelSteps);
	mesh.print();
        std::string fn = fileName + ".obj";
	mesh.saveOBJ(fn.c_str());
        fn = fileName + ".soup";
	mesh.saveSoup(fn.c_str());
        fn = fileName + ".txt";
	mesh.saveTXT(fn.c_str());
}
