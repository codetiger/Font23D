#include <ft2build.h>
#include <freetype.h>
#include <ftglyph.h>
#include <ftoutln.h>
#include <fttrigon.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <codecvt>
#include <locale>

#include "ftgl/Point.h"
#include "ftgl/Vectoriser.h"
#include "poly2tri/poly2tri.h"
#include "mesh.h"

using namespace p2t;
using namespace ftgl;

std::vector<p2t::Point*> triangulateContour(Vectoriser *vectoriser, int c, float offset) {
	std::vector<p2t::Point*> polyline;
	const Contour* contour = vectoriser->GetContour(c);
	for(size_t p = 0; p < contour->PointCount(); ++p) {
		const double* d = contour->GetPoint(p);
		polyline.push_back(new p2t::Point((d[0]/64.0f) + offset, d[1]/64.0f));
	}
	return polyline;
}

float AddCharacter(Mesh &mesh, FT_Face face, char32_t ch, unsigned short bezierSteps, float offset, float extrude) {
    static FT_UInt prevCharIndex = 0, curCharIndex = 0;
	static FT_Pos  prev_rsb_delta = 0;

    curCharIndex = FT_Get_Char_Index( face, ch );
    if(FT_Load_Glyph( face, curCharIndex, FT_LOAD_DEFAULT ))
        printf("FT_Load_Glyph failed\n");

    FT_Glyph glyph;
    if(FT_Get_Glyph( face->glyph, &glyph ))
        printf("FT_Get_Glyph failed\n");
 
    if(glyph->format != FT_GLYPH_FORMAT_OUTLINE) {
    	printf("Invalid Glyph Format\n");
    	exit(0);
    }

    short nCountour = 0;
    nCountour = face->glyph->outline.n_contours;

    int startIndex = 0, endIndex = 0;
	FT_Outline* o = &face->glyph->outline;

	if(FT_HAS_KERNING( face ) && prevCharIndex) {
		FT_Vector  kerning;
		FT_Get_Kerning( face, prevCharIndex, curCharIndex, FT_KERNING_DEFAULT, &kerning );
		offset += kerning.x >> 6;
	}

	if ( prev_rsb_delta - face->glyph->lsb_delta >= 32 )
		offset -= 1.0f;
	else if ( prev_rsb_delta - face->glyph->lsb_delta < -32 )
		offset += 1.0f;

	prev_rsb_delta = face->glyph->rsb_delta;
	Vertex v1, v2, v3;

    Vectoriser *vectoriser = new Vectoriser(face->glyph, bezierSteps);
	for(size_t c = 0; c < vectoriser->ContourCount(); ++c) {
		const Contour* contour = vectoriser->GetContour(c);
	
		for(size_t p = 0; p < contour->PointCount(); ++p) {
			ftgl::Point d1 = contour->GetPoint(p);
			ftgl::Point d2 = contour->GetPoint((p == contour->PointCount() - 1)?(0):(p + 1));
			d1 = d1 * (1.0f / 64.0f);
			d2 = d2 * (1.0f / 64.0f);

	    	v1.x = d1.X()+ offset;
			v1.y = d1.Y();
			v1.z = 0.0f;
	    	v2.x = d2.X() + offset;
			v2.y = d2.Y();
			v2.z = 0.0f;
	    	v3.x = d1.X() + offset;
			v3.y = d1.Y();
			v3.z = extrude;
	    	mesh.addTriangle(v1, v2, v3);

	    	v1.x = d1.X() + offset;
			v1.y = d1.Y();
			v1.z = extrude;
	    	v2.x = d2.X() + offset;
			v2.y = d2.Y();
			v2.z = 0.0f;
	    	v3.x = d2.X() + offset;
			v3.y = d2.Y();
			v3.z = extrude;
	    	mesh.addTriangle(v1, v2, v3);
		}

		if(contour->GetDirection()) {
			std::vector<p2t::Point*> polyline = triangulateContour(vectoriser, c, offset);
			CDT* cdt = new CDT(polyline);

			for(size_t cm = 0; cm < vectoriser->ContourCount(); ++cm) {
				const Contour* sm = vectoriser->GetContour(cm);
				if(c != cm && !sm->GetDirection() && sm->IsInside(contour)) {
					std::vector<p2t::Point*> pl = triangulateContour(vectoriser, cm, offset);
					cdt->AddHole(pl);
				}
			}

			cdt->Triangulate();
			std::vector<Triangle*> ts = cdt->GetTriangles();
			for(int i = 0; i < ts.size(); i++) {
				Triangle* ot = ts[i];

				v1.x = ot->GetPoint(0)->x;
				v1.y = ot->GetPoint(0)->y;
				v1.z = 0.0f;
				v2.x = ot->GetPoint(1)->x;
				v2.y = ot->GetPoint(1)->y;
				v2.z = 0.0f;
				v3.x = ot->GetPoint(2)->x;
				v3.y = ot->GetPoint(2)->y;
				v3.z = 0.0f;
				mesh.addTriangle(v1, v2, v3);

				v1.x = ot->GetPoint(0)->x;
				v1.y = ot->GetPoint(0)->y;
				v1.z = extrude;
				v2.x = ot->GetPoint(1)->x;
				v2.y = ot->GetPoint(1)->y;
				v2.z = extrude;
				v3.x = ot->GetPoint(2)->x;
				v3.y = ot->GetPoint(2)->y;
				v3.z = extrude;
				mesh.addTriangle(v3, v2, v1);
		    }
			delete cdt;
		}
	}

    delete vectoriser;
    vectoriser = NULL;

    prevCharIndex = curCharIndex;
    float chSize = face->glyph->advance.x >> 6;
    return offset + chSize;
}

Mesh generateMesh(char* fontFile, int height, std::u32string str, unsigned short bezierSteps, float extrude) {
    FT_Library library;
    if (FT_Init_FreeType( &library ))
        printf("FT_Init_FreeType failed\n");

    FT_Face face;
    if (FT_New_Face( library, fontFile, 0, &face ))
        printf("FT_New_Face failed (there is probably a problem with your font file\n");
 
    FT_Set_Char_Size( face, height << 6, height << 6, 96, 96);

	Mesh mesh;
   	float offset = 0; 
	for(char32_t ch : str) {
       	offset = AddCharacter(mesh, face, ch, bezierSteps, offset, extrude);
	}

	return mesh;
}

int main(int argc, char **argv) {
	char* fontName = argv[1];
	printf("Font: %s\n", fontName);
	int height = atoi(argv[3]);
	printf("Height: %d\n", height);
	unsigned short bezierSteps = atoi(argv[4]);
	printf("Bezier Steps: %d\n", bezierSteps);
	float extrude = atof(argv[5]);
	printf("Extrude: %f\n", extrude);
	char* objFileName = argv[6];
	printf("OBJ Filename: %s\n", objFileName);

	std::string strUtf8 = argv[2];
	printf("String: %s\n", strUtf8.c_str());
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    std::u32string str = conv.from_bytes(strUtf8);
	Mesh mesh = generateMesh(fontName, height, str, bezierSteps, extrude);
	mesh.print();
	mesh.saveOBJ(objFileName);
}

