#include <vector>
#include <iostream>
#include <sstream>

struct Vertex {
	float x, y, z;
    float nx, ny, nz;

    bool operator==(const Vertex& other) const {
        return x == other.x && y == other.y && z == other.z &&
                nx == other.nx && ny == other.ny && nz == other.nz;
    }

    Vertex operator-(const Vertex& b) const {
        Vertex r;
        r.x = x - b.x;
        r.y = y - b.y;
        r.z = z - b.z;
        return r;
    }

    void computeNormal(Vertex a, Vertex b, Vertex c) {
        Vertex e1 = b - a;
        Vertex e2 = c - a;
        nx = e1.y * e2.z - e1.z * e2.y;
        ny = e1.z * e2.x - e1.x * e2.z;
        nz = e1.x * e2.y - e1.y * e2.x;

        double norm = sqrt(nx*nx + ny*ny + nz*nz);
        nx /= norm;
        ny /= norm;
        nz /= norm;
    }
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<int> indices;

    int addVertex(Vertex v) {
        std::vector<Vertex>::iterator itr = std::find(vertices.begin(), vertices.end(), v);
        int index = 0;

        if (itr != vertices.cend()) {
            index = (int)std::distance(vertices.begin(), itr) + 1;
        } else {
            vertices.push_back(v);
            index = (int)vertices.size();
        }

        return index;
    }

    void addTriangle(Vertex a, Vertex b, Vertex c) {
        a.computeNormal(a, b, c);
        c.nx = b.nx = a.nx; 
        c.ny = b.ny = a.ny; 
        c.nz = b.nz = a.nz;

        indices.push_back(addVertex(c));
        indices.push_back(addVertex(b));
        indices.push_back(addVertex(a));
    }

    void print(){
        printf("Vertex Count: %d\n", (int)vertices.size());
        printf("Index Count: %d\n", (int)indices.size());
    }

    void saveOBJ(const char* filename) {
        std::ofstream objFile(filename);

        for (int i = 0; i < vertices.size(); i++)
            objFile << "v " << vertices[i].x << " " << vertices[i].y << " " << vertices[i].z << "\n";

        for (int i = 0; i < indices.size(); i+=3)
            objFile << "f " << indices[i] << " " << indices[i+1] << " " << indices[i+2] << "\n";

        objFile.close();
    }

    std::string getOBJData() {
        std::stringstream objData("");

        for (int i = 0; i < vertices.size(); i++)
            objData << "v " << vertices[i].x << " " << vertices[i].y << " " << vertices[i].z << "\n";

        for (int i = 0; i < indices.size(); i+=3)
            objData << "f " << indices[i] << " " << indices[i+1] << " " << indices[i+2] << "\n";

        return objData.str();
    }

};