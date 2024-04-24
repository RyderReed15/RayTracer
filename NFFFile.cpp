#include "NFFFile.h"


NFFFile::NFFFile(const std::string& szFilePath){
    m_cBackground = { 0, 0, 0 };

    std::ifstream fNFF;
    fNFF.open(szFilePath, std::ios::binary | std::ios::ate);

    if (fNFF.is_open()) {

        size_t size = fNFF.tellg();
        if (size) {
            fNFF.seekg(0, std::ios::beg);
            char* pBuffer = new char[size];
            char* pOrig = pBuffer;
            fNFF.read(pBuffer, size);
            fNFF.close();
            context = &pBuffer;
            ParseBuffer(pBuffer);
            delete[] pOrig;
        }
        else {

            fNFF.close();
            throw std::runtime_error("NFF File is empty!");
        }
    }
    else {
        throw std::runtime_error("Failed to open NFF File!");
    }
}

NFFFile::~NFFFile(void){
    for (auto& e : m_vShapes) {
        delete e;
    }
}


Color NFFFile::GetBackground(void) const {
    return m_cBackground;
}

Viewpoint NFFFile::GetViewpoint(void) const {
    return m_vView;
}

std::vector<Shape*> NFFFile::GetShapes(void) const {
    return m_vShapes;
}

std::vector<Light> NFFFile::GetLights(void) const{
    return m_vLights;
}



void NFFFile::ParseBuffer(char* pBuffer){

    Material cCurrentMaterial;
    char* pToken = strtok_s(pBuffer, " \n\r", context);
    while (pToken) {
        switch (*pToken) {
            case 'b':
                //Handle background color
                ParseBackgroundColor();

                break;
            case 'v':
                //Handle Viewpoint
                m_vView = ParseViewpoint();

                break;
            case 'f':
                // Handle Fill
                cCurrentMaterial = ParseMaterial();

                break;
            case 'p':
                //Handle Polygon
                
                ParsePolygon(cCurrentMaterial, strcmp(pToken, "pp") == 0);

                break;
            case 's':
                //Handle Spheres
                ParseSphere(cCurrentMaterial);

                break;
            case 'l':
                //Handle Lights
                ParseLight();

                break;
            default:
                break;
        }
        pToken = strtok_s(0, " \n\r", context);
    }
}

void NFFFile::ParseBackgroundColor(){
    m_cBackground = {
        strtof(strtok_s(0, " \n\r", context), 0), // r
        strtof(strtok_s(0, " \n\r", context), 0), // g
        strtof(strtok_s(0, " \n\r", context), 0)  // b
    };
}

Viewpoint NFFFile::ParseViewpoint(){
    char* pToken = strtok_s(0, " \n\r", context);
    float from[3] = { 
        strtof(strtok_s(0, " \n\r", context), 0),
        strtof(strtok_s(0, " \n\r", context), 0),
        strtof(strtok_s(0, " \n\r", context), 0) };

    pToken = strtok_s(0, " \n\r", context);
    float at[3] = {
        strtof(strtok_s(0, " \n\r", context), 0),
        strtof(strtok_s(0, " \n\r", context), 0),
        strtof(strtok_s(0, " \n\r", context), 0) };

    pToken = strtok_s(0, " \n\r", context);
    float up[3] = {
        strtof(strtok_s(0, " \n\r", context), 0),
        strtof(strtok_s(0, " \n\r", context), 0),
        strtof(strtok_s(0, " \n\r", context), 0) };

    pToken = strtok_s(0, " \n\r", context);
    float angle = strtof(strtok_s(0, " \n\r", context), 0);

    pToken = strtok_s(0, " \n\r", context);
    float hither = strtof(strtok_s(0, " \n\r", context), 0);

    pToken = strtok_s(0, " \n\r", context);
    int x = strtol(strtok_s(0, " \n\r", context), 0, 10);
    int y = strtol(strtok_s(0, " \n\r", context), 0, 10);

    
    return { 
        {from[0], from[1], from[2] }, 
        {at[0], at[1], at[2] },
        {up[0], up[1], up[2] }, 
        angle, hither, y, x
    };
    
}

void NFFFile::ParseLight(){
    m_vLights.push_back({
        {
            strtof(strtok_s(0, " \n\r", context), 0), // x
            strtof(strtok_s(0, " \n\r", context), 0), // y
            strtof(strtok_s(0, " \n\r", context), 0)  // z
        }, // pos
        {1,1,1}
       
    });
}

Material NFFFile::ParseMaterial() {
    return {
        {
            strtof(strtok_s(0, " \n\r", context), 0), // r
            strtof(strtok_s(0, " \n\r", context), 0), // g
            strtof(strtok_s(0, " \n\r", context), 0)  // b
        },
        strtof(strtok_s(0, " \n\r", context), 0), // Kd
        strtof(strtok_s(0, " \n\r", context), 0), // Ks
        strtof(strtok_s(0, " \n\r", context), 0), // shine
        strtof(strtok_s(0, " \n\r", context), 0), // t
        strtof(strtok_s(0, " \n\r", context), 0) // index of refraction
    };
}


void NFFFile::ParsePolygon(const Material& cCurrColor, bool bPatch){
    int iVertCount = strtol(strtok_s(0, " \n\r", context), 0, 10);

    std::vector<Eigen::Vector3f> vVertices;
    std::vector<Eigen::Vector3f> vNormals;

    for (int i = 0; i < iVertCount; i++){
       vVertices.push_back({
            strtof(strtok_s(0, " \n\r", context), 0), // r
            strtof(strtok_s(0, " \n\r", context), 0), // g
            strtof(strtok_s(0, " \n\r", context), 0)  // b
           });
       if(bPatch) vNormals.push_back({
            strtof(strtok_s(0, " \n\r", context), 0), // r
            strtof(strtok_s(0, " \n\r", context), 0), // g
            strtof(strtok_s(0, " \n\r", context), 0)  // b
           });
    }


    //Shape is triangle
    if(vVertices.size() == 3){
        if (bPatch) {
            m_vShapes.push_back(new Triangle(vVertices[0], vVertices[1], vVertices[2], vNormals[0], vNormals[1], vNormals[2], cCurrColor));
        }
        else {
            m_vShapes.push_back(new Triangle(vVertices[0], vVertices[1], vVertices[2], cCurrColor));
        }
        
        return;
    }
    
    //Determine if shape is convex or concave
    //might not be needed as intersecting a bunch of triangles is slower than a single polygon without bvh
    auto n = (vVertices[1] - vVertices[0]).cross(vVertices[2] - vVertices[0]);
    auto n2 = n.cross(vVertices[1] - vVertices[0]);
    bool dir = n2.dot(vVertices[2] - vVertices[1]) >= 0;

    for (int i = 2; i < vVertices.size() - 1; i++) {
        if (dir != (n2.dot(vVertices[i+1] - vVertices[i]) >= 0)) {
            if(bPatch) m_vShapes.push_back(new Polygon(vVertices, vNormals, cCurrColor));
            else m_vShapes.push_back(new Polygon(vVertices, cCurrColor));
            return;
        }
    }
    for (int i = 1; i < vVertices.size()-1; i++) {
        if (bPatch) {
            m_vShapes.push_back(new Triangle(vVertices[0], vVertices[i], vVertices[i + 1], vNormals[0], vNormals[i], vNormals[i + 1], cCurrColor));
        }
        else {
            m_vShapes.push_back(new Triangle(vVertices[0], vVertices[i], vVertices[i + 1], cCurrColor));
        }
        
    }

}

void NFFFile::ParseSphere(const Material& cCurrColor) {
   
    m_vShapes.push_back(new Sphere(
        {
            strtof(strtok_s(0, " \n\r", context), 0), // x
            strtof(strtok_s(0, " \n\r", context), 0), // y
            strtof(strtok_s(0, " \n\r", context), 0)  // z
        }, // vCenter
        strtof(strtok_s(0, " \n\r", context), 0),  // radius
        cCurrColor
    ));
    
}

