#pragma once

#ifndef NFF_FILE_H
#define NFF_FILE_H

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "Structs.h"
#include "Shape.h"




class NFFFile {
public:
    NFFFile (const std::string& szFilePath);
    ~NFFFile(void);

    Color                   GetBackground   (void) const;
    Viewpoint               GetViewpoint    (void) const;
    std::vector<Shape*>     GetShapes       (void) const;
    std::vector<Light>      GetLights       (void) const;

private:

    void        ParseBuffer         (char* pBuffer);
    void        ParseBackgroundColor(void);
    Viewpoint   ParseViewpoint      (void);
    void        ParseLight          (void);
    Material    ParseMaterial       (void);
    void        ParsePolygon        (const Material& cCurrColor, bool bPatch = false);
    void        ParseSphere         (const Material& cCurrColor);
    
    Color                   m_cBackground;
    Viewpoint               m_vView;

    std::vector<Shape*>     m_vShapes;
    std::vector<Light>      m_vLights;

    char** context;

};


#endif // !NFF_FILE_H