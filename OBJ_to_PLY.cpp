#include <iostream> 
#include <fstream> 
#include <sstream>
#include <string> 
#include <vector> 

struct Vec3 {

    float x, y, z;
};

struct Face {

    std::vector<int> vertexIndices;
};

int main() {

    //create a input stream file called objFile to read 3DModel_Custom_copy.obj
    std::ifstream objFile("3DModel.obj");

    //see if the file could be open or not. If not give an error.
    if (!objFile.is_open()) {

        std::cerr << "Could not open 3DModel_Custom_copy.obj" << std::endl;
        return 1;
    }
    
    std::vector<Vec3> vertices;
    std::vector<Face> faces;

    std::string line;

    //read each line of the 3DModel_Custom_copy obj file
    while (std::getline(objFile, line)) {

        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix; 

        // std::cout << prefix << std::endl;

        if (prefix == "v") {

            Vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            //add vertex to a list of vertices 
            vertices.push_back(vertex);

        } else if (prefix == "f") {

            Face face;
            std::string token; 
            
            //parse the string such that I can get  
            while (iss >> token) {
                std::istringstream tokenStream(token);
                std::string indexStr;
                std::getline(tokenStream, indexStr, '/'); // Get vertex index (ignore texture/normal)
                int vertexIndex = std::stoi(indexStr) - 1; // OBJ uses 1-based indexing
                std::cout << "Face_vIndex: " << vertexIndex << std::endl; 
                face.vertexIndices.push_back(vertexIndex);
            }
            faces.push_back(face);

            
        }
    }
    
    //this is after storing all the faces and vertices on vectors
    objFile.close();
    
        // Write to PLY file
    std::ofstream plyFile("3DModel_Custom_copy.ply");
    if (!plyFile.is_open()) {
        std::cerr << "Could not create mesh.ply" << std::endl;
        return 1;
    }

    // Write PLY header
    plyFile << "ply\n";
    plyFile << "format ascii 1.0\n";
    plyFile << "element vertex " << vertices.size() << "\n";
    plyFile << "property float x\n";
    plyFile << "property float y\n";
    plyFile << "property float z\n";
    plyFile << "element face " << faces.size() << "\n";
    plyFile << "property list uchar int vertex_indices\n";
    plyFile << "end_header\n";

    // Write vertex data
    for (const auto& v : vertices) {
        plyFile << v.x << " " << v.y << " " << v.z << "\n";
    }

    // Write face data
    for (const auto& f : faces) {
        plyFile << f.vertexIndices.size();
        for (int idx : f.vertexIndices) {
            plyFile << " " << idx;
        }
        plyFile << "\n";
    }

    plyFile.close();
    std::cout << "PLY file written to 3DModel.ply" << std::endl;
    
    
    // return success
    return 0;
}