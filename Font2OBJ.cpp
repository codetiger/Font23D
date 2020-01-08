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

#include <GL/glut.h>

#include "ftgl/Point.h"
#include "ftgl/Vectoriser.h"
#include "poly2tri/poly2tri.h"

using namespace p2t;

int USE_OPENGL = 0;

struct Vector3df
{
	float x, y, z;
};

struct Tri
{
	Vector3df a, b, c;
};

std::vector<Tri> tris;

GLfloat light_diffuse[] = {1.0, 1.0, 1.0, 1.0};  /* Red diffuse light. */
GLfloat light_position[] = {1.0, 1.0, 1.0, 0.0};  /* Infinite light location. */

void initGL(void)
{
	/* Enable a single OpenGL light. */
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glFrontFace(GL_CW); 

	glMatrixMode(GL_PROJECTION);
	gluPerspective( /* field of view in degree */ 40.0, /* aspect ratio */ 1.0,	/* Z near */ 1.0, /* Z far */ 1000.0);
	glMatrixMode(GL_MODELVIEW);
	gluLookAt(0.0, 0.0, 500.0,  /* eye is at (0,0,5) */ 0.0, 0.0, 0.0,      /* center is at (0,0,0) */ 0.0, 1.0, 0.); /* up is in positive Y direction */
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glRotatef(1.0f, 0.0f, 0.0f, 0.0f);

	for (int i = 0; i < tris.size(); i++) {
		Tri t = tris[i];
		glBegin(GL_TRIANGLES);
		glVertex3f(t.a.x, t.a.y, t.a.z);
		glVertex3f(t.b.x, t.b.y, t.b.z);
		glVertex3f(t.c.x, t.c.y, t.c.z);
		glEnd();
	}
	glutSwapBuffers();
    glutPostRedisplay();
}

std::vector<p2t::Point*> triangulateContour(Vectoriser *vectoriser, int c, float offset) {
	std::vector<p2t::Point*> polyline;
	const Contour* contour = vectoriser->GetContour(c);
	for(size_t p = 0; p < contour->PointCount(); ++p) {
		const double* d = contour->GetPoint(p);
		polyline.push_back(new p2t::Point((d[0]/64.0f) + offset, d[1]/64.0f));
	}
	return polyline;
}

void beginCallback(GLenum which)
{
	printf("Unknown tesselasion enum: %d\n", which);
	glBegin(GL_TRIANGLES);
}

void endCallback(void)
{
	glEnd();
}

void flagCallback( GLboolean ) 
{

}

void errorCallback(GLenum errorCode)
{
   const GLubyte *estring;

   estring = gluErrorString(errorCode);
   fprintf (stderr, "Tessellation Error: %s\n", estring);
}

static int count = 0;
static Tri t;

void vertexCallback(GLvoid *vertex)
{
	const GLdouble *d;
	d = (GLdouble *) vertex;

	if(count == 0) {
		t.a.x = d[0];
		t.a.y = d[1];
		t.a.z = d[2];
		count++;
	} else if(count == 1) {
		t.c.x = d[0];
		t.c.y = d[1];
		t.c.z = d[2];
		count++;
	} else if(count == 2) {
		t.b.x = d[0];
		t.b.y = d[1];
		t.b.z = d[2];
		tris.push_back(t);
		count = 0;
		printf("Creating triangles\n");
	}
}

void combineCallback(GLdouble coords[3], GLdouble *vertex_data[4], GLfloat weight[4], GLdouble **dataOut)
{
	printf("combineCallback\n");
	GLdouble *vertex;
	int i;

	vertex = (GLdouble *) malloc(6 * sizeof(GLdouble));
	vertex[0] = coords[0];
	vertex[1] = coords[1];
	vertex[2] = coords[2];
	for (i = 3; i < 7; i++)
		vertex[i] = weight[0] * vertex_data[0][i] + weight[1] * vertex_data[1][i] + weight[2] * vertex_data[2][i] + weight[3] * vertex_data[3][i];
	*dataOut = vertex;
}


float AddCharacter(FT_Face face, FT_ULong ch, unsigned short bezierSteps, float offset, float extrude) {
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

    Vectoriser *vectoriser = new Vectoriser(face->glyph, bezierSteps);
	for(size_t c = 0; c < vectoriser->ContourCount(); ++c) {
		const Contour* contour = vectoriser->GetContour(c);

		for(size_t p = 0; p < contour->PointCount() - 1; ++p) {
			const double* d1 = contour->GetPoint(p);
			const double* d2 = contour->GetPoint(p + 1);
	    	Tri t1;
	    	t1.a.x = (d1[0]/64.0f) + offset;
			t1.a.y = d1[1]/64.0f;
			t1.a.z = 0.0f;
	    	t1.b.x = (d2[0]/64.0f) + offset;
			t1.b.y = d2[1]/64.0f;
			t1.b.z = 0.0f;
	    	t1.c.x = (d1[0]/64.0f) + offset;
			t1.c.y = d1[1]/64.0f;
			t1.c.z = extrude;
	    	tris.push_back(t1);

            Tri t2;
            t2.a.x = (d1[0]/64.0f) + offset;
            t2.a.y = d1[1]/64.0f;
            t2.a.z = extrude;
            t2.c.x = (d2[0]/64.0f) + offset;
            t2.c.y = d2[1]/64.0f;
            t2.c.z = extrude;
            t2.b.x = (d2[0]/64.0f) + offset;
            t2.b.y = d2[1]/64.0f;
            t2.b.z = 0.0f;

	    	tris.push_back(t2);
		}

		if(contour->GetDirection()) {

		    if(USE_OPENGL) {
				GLUtesselator* tobj = gluNewTess();

				gluTessCallback(tobj, GLU_TESS_VERTEX, (GLvoid (*) ()) &vertexCallback);
				gluTessCallback(tobj, GLU_TESS_BEGIN, (GLvoid (*) ()) &beginCallback);
				gluTessCallback(tobj, GLU_TESS_END, (GLvoid (*) ()) &endCallback);
				gluTessCallback(tobj, GLU_TESS_ERROR, (GLvoid (*) ()) &errorCallback);
				gluTessCallback(tobj, GLU_TESS_COMBINE, (GLvoid (*) ()) &combineCallback);
			    gluTessCallback(tobj, GLU_TESS_EDGE_FLAG, (GLvoid (*) ()) &flagCallback);

				gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);

				gluTessProperty(tobj, GLU_TESS_TOLERANCE, 0);
				gluTessNormal(tobj, 0.0f, 0.0f, 0.0f);

				gluTessBeginPolygon(tobj, NULL);
				for(size_t c = 0; c < vectoriser->ContourCount(); ++c) {
					const Contour* contour = vectoriser->GetContour(c);
					gluTessBeginContour(tobj);
					for(size_t p = 0; p < contour->PointCount(); ++p) {
						const double* d1 = contour->GetPoint(p);
						double *d = new double[3];
						d[0] = d1[0] / 64.0f + offset;
						d[1] = d1[1] / 64.0f;
						d[2] = d1[2] / 64.0f;
						gluTessVertex(tobj, (GLdouble *)d, (GLvoid *)d);
					}

					gluTessEndContour(tobj);
				}
				gluTessEndPolygon(tobj);

		    } else {
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

			    	Tri t1;
			    	t1.a.x = ot->GetPoint(0)->x;
			    	t1.a.y = ot->GetPoint(0)->y;
			    	t1.a.z = 0.0f;
			    	t1.b.x = ot->GetPoint(1)->x;
			    	t1.b.y = ot->GetPoint(1)->y;
			    	t1.b.z = 0.0f;
			    	t1.c.x = ot->GetPoint(2)->x;
			    	t1.c.y = ot->GetPoint(2)->y;
			    	t1.c.z = 0.0f;
			    	tris.push_back(t1);

			    	Tri t2;
			    	t2.a.x = ot->GetPoint(1)->x;
			    	t2.a.y = ot->GetPoint(1)->y;
			    	t2.a.z = extrude;
			    	t2.b.x = ot->GetPoint(0)->x;
			    	t2.b.y = ot->GetPoint(0)->y;
			    	t2.b.z = extrude;
			    	t2.c.x = ot->GetPoint(2)->x;
			    	t2.c.y = ot->GetPoint(2)->y;
			    	t2.c.z = extrude;
			    	tris.push_back(t2);
			    }
			    delete cdt;
		    }
		}
	}

    delete vectoriser;
    vectoriser = NULL;

    prevCharIndex = curCharIndex;
    float chSize = face->glyph->advance.x >> 6;
    return offset + chSize;
}

int main(int argc, char **argv) {
	int height = atoi(argv[3]);
	char* strUtf8 = argv[2];
	const auto str = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}.from_bytes(strUtf8);
	unsigned short bezierSteps = atoi(argv[4]);
	float extrude = atof(argv[5]);

    FT_Library library;
    if (FT_Init_FreeType( &library ))
        printf("FT_Init_FreeType failed\n");

    FT_Face face;
    if (FT_New_Face( library, argv[1], 0, &face ))
        printf("FT_New_Face failed (there is probably a problem with your font file\n");
 
    FT_Set_Char_Size( face, height << 6, height << 6, 96, 96);

   	float offset = 0; 
    for(int i = 0; i < str.size(); i++) {
       	offset = AddCharacter(face, str[i], bezierSteps, offset, extrude);
    }

    // Debug Info
    printf("Point Count: %d\n", (int)tris.size());
	for (int i = 0; i < tris.size(); i++) {
		Tri t = tris[i];
	}

	// Draw
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize (1024, 640);
	glutInitWindowPosition (100, 100);

	glutCreateWindow("Font to 3D");
	glutDisplayFunc(display);
	initGL();

	glTranslatef(-100.0f, 0.0f, 0.0f);
	glRotatef(0.0f, 1.0f, 0.0f, 0.0f);

	glutMainLoop();
}

