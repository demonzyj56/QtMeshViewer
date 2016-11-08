//
// Created by leoyolo on 9/9/16.
//

#ifndef OPENGLPLAYGROUND_MPARSER_H
#define OPENGLPLAYGROUND_MPARSER_H

#include "common.h"
#include "TriMesh.h"
#include <memory>
#include <string>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <fstream>

std::shared_ptr<TriMesh> ReadMFile(const std::string &filename) {
    std::ifstream m_file(filename, std::ios_base::in);
    if (!m_file.is_open()) {
        printf("ReadMFile: Cannot read mfile %s.\n", filename.c_str());
        return nullptr;
    }
    auto m_mesh = std::make_shared<TriMesh>();
    std::string line;
    int id;
    float vert_coord[3];
    int vert_id[3];
    // char name[10];
    std::string kind;
    tic();
    while(std::getline(m_file, line)) {
        if (line[0] == '#') continue;   // TODO: get rid of whitespaces
        else {
            std::stringstream ss(line, std::ios_base::in);
            ss >> kind;
            if (kind == "Vertex") {
                ss >> id >> vert_coord[0] >> vert_coord[1] >> vert_coord[2];
                m_mesh->InsertVertex(vert_coord[0], vert_coord[1], vert_coord[2], id);
                // printf("Vertex: %f %f %f\n", vert_coord[0], vert_coord[1], vert_coord[2]);
            }
            else if (kind == "Face") {
                ss >> id >> vert_id[0] >> vert_id[1] >> vert_id[2];
                m_mesh->InsertFace(id, vert_id[0], vert_id[1], vert_id[2]);
            }
            else {
                printf("ReadMFile: Unknown parse format:\n");
                printf("%s\n", line.c_str());
                continue;
            }
        }
    }
    m_mesh->Update();
    toc();
    return m_mesh;
}



#endif //OPENGLPLAYGROUND_MPARSER_H
