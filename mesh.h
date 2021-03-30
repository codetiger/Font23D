#include <vector>
#include <iostream>

struct Vertex {
	float x, y, z;

    bool operator==(const Vertex& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<int> indices;
    int indexCount = 0;

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
        indices.push_back(addVertex(c));
        indices.push_back(addVertex(b));
        indices.push_back(addVertex(a));
    }

    void print(){
        printf("Vertex Count: %d\n", (int)vertices.size());
        printf("Index Count: %d\n", (int)indices.size());
    }

    void saveOBJ(char* filename) {
        std::ofstream objFile(filename);

        for (int i = 0; i < vertices.size(); i++)
            objFile << "v " << vertices[i].x << " " << vertices[i].y << " " << vertices[i].z << "\n";

        for (int i = 0; i < indices.size(); i+=3)
            objFile << "f " << indices[i] << " " << indices[i+1] << " " << indices[i+2] << "\n";

        objFile.close();
    }
};