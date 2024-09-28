#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v.raw[i];
            verts_.push_back(v);
        }
        else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            Vec2f uv;
            for (int i = 0; i < 2; i++) {
                iss >> uv[i];
            }
            uv_.push_back({uv.x, 1 - uv.y});
        }
        else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            Vec3f normal;
            for (int i = 0; i < 3; i++) {
                iss >> normal[i];
            }
            norms_.push_back(normal);
        }
        else if (!line.compare(0, 2, "f ")) {
            int v, t, n;
            iss >> trash;
            int cnt = 0;
            while (iss >> v >> trash >> t >> trash >> n) {
                faceVrts_.push_back(--v);
                faceTexs_.push_back(--t);
                faceNrms_.push_back(--n);
                cnt++;
            }
            if (3 != cnt) {
                std::cerr << "Error: the obj file is supposed to be triangulated" << std::endl;
                return;
            }
        }
    }
    std::cerr << "# v# " << nverts() << " f# " << nfaces() 
        << " vt# " << uv_.size() << " vn# " << norms_.size() << std::endl;
    loadTexture(filename, "_diffuse.tga", diffuseMap_);
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faceVrts_.size() / 3;
}

Vec2f Model::uv(int iface, int nvert)
{
    return{ uv_[faceTexs_[iface * 3 + nvert]].x * diffuseMap_.get_width(), uv_[faceTexs_[iface * 3 + nvert]].y * diffuseMap_.get_height() };
}

Vec3i Model::face(int iface)
{
    return Vec3i{ faceVrts_[iface*3], faceVrts_[iface * 3 + 1], faceVrts_[iface * 3 + 2] };
}

TGAColor Model::diffuse(Vec2f uv)
{
    return diffuseMap_.get(uv.x, uv.y);
}

Vec3f Model::vert(int iface, int nvert)
{
    return verts_[faceVrts_[iface * 3 + nvert]];
}

void Model::loadTexture(const std::string& filename, const std::string& suffix, TGAImage& image)
{
    size_t dot = filename.find_last_of(".");
    std::string texfile = filename.substr(0, dot) + suffix;
    std::cerr << "texture file " << texfile << " loading "
        << (image.read_tga_file(texfile.c_str()) ? "ok" : "failed")
        << std::endl;
}
