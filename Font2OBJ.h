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

Vectoriser* getVectoriser(FT_Face face, wchar_t ch, unsigned short bezierSteps, double strength)
{
    FT_UInt charIndex = FT_Get_Char_Index(face, ch);
    if(FT_Load_Glyph(face, charIndex, FT_LOAD_DEFAULT)) {
        printf("Error Loading Char from Font");
        return NULL;
    }
    
    FT_Glyph glyph;
    if(FT_Get_Glyph(face->glyph, &glyph)) {
        printf("Error Loading Char from Font");
        return NULL;
    }
    
    if(glyph->format != FT_GLYPH_FORMAT_OUTLINE) {
        printf("Error Loading Char from Font");
        return NULL;
    }
    
    if(strength != 0.0) {
        FT_Outline_Embolden(&face->glyph->outline, strength * 64.0);
        FT_Outline_Translate(&face->glyph->outline, -strength * 64.0 * 0.5, -strength * 64.0 * 0.5);
    }
    FT_Orientation f = FT_Outline_Get_Orientation(&face->glyph->outline);
    
    Vectoriser *v = new Vectoriser(face->glyph->outline, bezierSteps, f==true);
    FT_Done_Glyph(glyph);
    
    return v;
}

float AddCharacter(Mesh &mesh, FT_Face face, char32_t ch, unsigned short bezierSteps, float offset, float extrude, float bevelRadius, unsigned short bevelSegments) {
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
	FT_Orientation f = FT_Outline_Get_Orientation(&face->glyph->outline);

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
    
    Vectoriser *vectoriser = getVectoriser(face, ch, bezierSteps, 0.0f);
	for(size_t c = 0; c < vectoriser->ContourCount(); ++c) {
		const Contour* contour = vectoriser->GetContour(c);

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

				Vertex v1, v2, v3;
				v1.x = ot->GetPoint(0)->x;
				v1.y = ot->GetPoint(0)->y;
				v1.z = -bevelRadius;
				v2.x = ot->GetPoint(1)->x;
				v2.y = ot->GetPoint(1)->y;
				v2.z = -bevelRadius;
				v3.x = ot->GetPoint(2)->x;
				v3.y = ot->GetPoint(2)->y;
				v3.z = -bevelRadius;
				mesh.addTriangle(v1, v2, v3);

				v1.x = ot->GetPoint(0)->x;
				v1.y = ot->GetPoint(0)->y;
				v1.z = extrude+bevelRadius;
				v2.x = ot->GetPoint(1)->x;
				v2.y = ot->GetPoint(1)->y;
				v2.z = extrude+bevelRadius;
				v3.x = ot->GetPoint(2)->x;
				v3.y = ot->GetPoint(2)->y;
				v3.z = extrude+bevelRadius;
				mesh.addTriangle(v3, v2, v1);
		    }
			delete cdt;
		}
	}
    delete vectoriser;
    vectoriser = NULL;

    for (int i = 0; i < bevelSegments; i++) {
        double bevelX = bevelRadius * 1.5 * sinf(M_PI * 0.5 * i / (double)bevelSegments);
        double nextBevelX = bevelRadius * 1.5  * sinf(M_PI * 0.5 * (i+1) / (double)bevelSegments);
        
        Vectoriser* vectoriser1 = getVectoriser(face, ch, bezierSteps, bevelX);
        Vectoriser* vectoriser2 = getVectoriser(face, ch, bezierSteps, nextBevelX);
        
        for(size_t c = 0; c < vectoriser1->ContourCount(); c++) {
            const Contour* contour1 = vectoriser1->GetContour(c);
            const Contour* contour2 = vectoriser2->GetContour(c);
            
            for(size_t p = 0; p < contour1->PointCount(); p++) {
				ftgl::Point d1 = contour1->GetPoint(p);
				ftgl::Point d2 = contour1->GetPoint((p == contour1->PointCount() - 1)?(0):(p + 1));
				d1 = d1 * (1.0f / 64.0f);
				d2 = d2 * (1.0f / 64.0f);

				ftgl::Point d3 = contour2->GetPoint(p);
				ftgl::Point d4 = contour2->GetPoint((p == contour2->PointCount() - 1)?(0):(p + 1));
				d3 = d3 * (1.0f / 64.0f);
				d4 = d4 * (1.0f / 64.0f);
                
                double bevelY = extrude + (bevelRadius * cosf(M_PI * 0.5 * (i) / (double)bevelSegments));
                double nextBevelY = extrude + (bevelRadius * cosf(M_PI * 0.5 * (i+1) / (double)bevelSegments));
                
				Vertex v1, v2, v3;
				v1.x = d1.X()+ offset;
				v1.y = d1.Y();
				v1.z = bevelY;
				v2.x = d3.X() + offset;
				v2.y = d3.Y();
				v2.z = nextBevelY;
				v3.x = d2.X() + offset;
				v3.y = d2.Y();
				v3.z = bevelY;
				mesh.addTriangle(v1, v2, v3);

				v1.x = d3.X()+ offset;
				v1.y = d3.Y();
				v1.z = nextBevelY;
				v2.x = d4.X() + offset;
				v2.y = d4.Y();
				v2.z = nextBevelY;
				v3.x = d2.X() + offset;
				v3.y = d2.Y();
				v3.z = bevelY;
				mesh.addTriangle(v1, v2, v3);

                double eBevelY = -(bevelRadius * cosf(M_PI * 0.5 * (i) / (double)bevelSegments));
                double eNextBevelY = -(bevelRadius * cosf(M_PI * 0.5 * (i+1) / (double)bevelSegments));
                
				v1.x = d1.X()+ offset;
				v1.y = d1.Y();
				v1.z = eBevelY;
				v2.x = d2.X() + offset;
				v2.y = d2.Y();
				v2.z = eBevelY;
				v3.x = d3.X() + offset;
				v3.y = d3.Y();
				v3.z = eNextBevelY;
				mesh.addTriangle(v1, v2, v3);

				v1.x = d3.X()+ offset;
				v1.y = d3.Y();
				v1.z = eNextBevelY;
				v2.x = d2.X() + offset;
				v2.y = d2.Y();
				v2.z = eBevelY;
				v3.x = d4.X() + offset;
				v3.y = d4.Y();
				v3.z = eNextBevelY;
				mesh.addTriangle(v1, v2, v3);
            }
        }
        delete vectoriser1;
        delete vectoriser2;
        vectoriser1 = NULL;
        vectoriser2 = NULL;
    }

	vectoriser = getVectoriser(face, ch, bezierSteps, bevelRadius*1.5f);
	for(size_t c = 0; c < vectoriser->ContourCount(); ++c) {
		const Contour* contour = vectoriser->GetContour(c);

		for(size_t p = 0; p < contour->PointCount(); ++p) {
			ftgl::Point d1 = contour->GetPoint(p);
			ftgl::Point d2 = contour->GetPoint((p == contour->PointCount() - 1)?(0):(p + 1));
			d1 = d1 * (1.0f / 64.0f);
			d2 = d2 * (1.0f / 64.0f);

			Vertex v1, v2, v3;
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
	}
    delete vectoriser;
    vectoriser = NULL;

    prevCharIndex = curCharIndex;
    float chSize = face->glyph->advance.x >> 6;
    return offset + chSize;
}

Mesh generateMesh(char* fontFile, int height, std::u32string str, unsigned short bezierSteps, float extrude, float bevelRadius, unsigned short bevelSteps) {
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
       	offset = AddCharacter(mesh, face, ch, bezierSteps, offset, extrude, bevelRadius, bevelSteps);
	}

	return mesh;
}
