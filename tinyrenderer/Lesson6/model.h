#ifndef TINY_RENDERER_MOLE_H
#define TINY_RENDERER_MOLE_H

#include <vector>
#include "geometry.h"
#include "tgaimage.h"

class Model {
private:
    std::vector<Vec3f> verts_{};
    std::vector<Vec3f> norms_{};
    std::vector<Vec2f> uv_{};

	std::vector<int> faceVrts_{};
    std::vector<int> faceTexs_{};
    std::vector<int> faceNrms_{};

    TGAImage diffuseMap_;

    void loadTexture(const std::string& filename, const std::string& suffix, TGAImage& image);

public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
    Vec3f norm(int iface, int nvert);
    Vec3f vert(int iface, int nvert);
    Vec2f uv(int iface, int nvert);
    Vec3i face(int iface);
    TGAColor diffuse(Vec2f uv);
};

#endif // !TINY_RENDERER_MOLE_H
