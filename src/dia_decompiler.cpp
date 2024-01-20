
#include "../include/yds_math.h"
#include "../include/ysce.h"
#include "../include/dia.h"
#include "../include/common.h"
#include "../include/yds_interchange_object.h"

#include <iostream>
#include <fstream>
#include <set>
#include <map>

union Convert2Float
{
	unsigned char byte[4];
	float real;
};

int main(int argc, char** argv)
{
	std::cout << "Opening file..." << std::endl;
	std::fstream file("assets.dia", std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		file.close();
		std::cerr << "Failed to open assets.dia file." << std::endl;
		return 1;
	}

	IdHeader idHeader;
	file.read((char*)&idHeader, sizeof(IdHeader));

	if (idHeader.MagicNumber != MAGIC_NUMBER) {
		file.close();
		std::cerr << "Invalid magic number" << std::endl;
		return 1;
	}

	if (idHeader.MinorVersion != MINOR_VERSION || idHeader.MajorVersion != MAJOR_VERSION) {
		file.close();
		std::cerr << "Invalid version" << std::endl;
		return 1;
	}

	SceneHeader sceneHeader;
	file.read((char*)&sceneHeader, sizeof(SceneHeader));

	for (int i = 0; i < sceneHeader.ObjectCount; i++) {
		ysInterchangeObject object = readobject(&file);
		VertexInfo vertexInfo;
		
		if (object.Type == ysInterchangeObject::ObjectType::Geometry) {
			if (vertexInfo.IncludeNormals) object.RipByNormals();
			if (vertexInfo.IncludeTangents) object.RipByTangents();
			if (vertexInfo.IncludeUVs) object.RipByUVs();
			std::cout << "New geometry object: " << object.Name << std::endl;

			// create filename
			char fname[64];
			strcpy(fname, object.Name.c_str());
			strcat(fname, ".obj");

			// open file
			std::fstream of(fname, std::ios::out);
			if (!of.is_open()) {
				of.close();
				file.close();
				std::cerr << "Failed to open " << fname << " file." << std::endl;
				return 1;
			}
			of << "# dia-decompiler exported file" << std::endl;
			of << "# made by DDev (@ddev on Discord)" << std::endl;
			of << "# object: " << object.Name << std::endl;
			of << "# vertex count: " << object.Vertices.size() << std::endl;
			of << "# face count: " << object.VertexIndices.size() << std::endl;
			of << "o " << object.Name << std::endl;

			// read vertices
			for (int j = 0; j < object.Vertices.size(); j++) {
				float x, y, z;
				x = object.Vertices[j].x;
				y = object.Vertices[j].y;
				z = object.Vertices[j].z;
				//std::cout << "x: " << x << " y: " << y << " z: " << z << std::endl;
				of << "v " << x << " " << y << " " << z << std::endl;

				if (object.UVChannels.size() > 0) {
					ysVector2 uv = object.UVChannels[0].Coordinates[0];
					of << "vt " << uv.x << " " << uv.y << std::endl;
				}

				float xn, yn, zn;
				if (object.NormalIndices.size() > j) {
					xn = object.NormalIndices[j].x;
					yn = object.NormalIndices[j].y;
					zn = object.NormalIndices[j].z;
				}
				else {
					xn = object.NormalIndices[0].x;
					yn = object.NormalIndices[0].y;
					zn = object.NormalIndices[0].z;
				}
				of << "vn " << x << " " << y << " " << z << std::endl;
			}

			// read indices
			for (int j = 0; j < object.VertexIndices.size(); j++) {
				int off = j * 3;
				int v1, v2, v3;
				v1 = object.VertexIndices[j].indices[0] + 1;
				v2 = object.VertexIndices[j].indices[1] + 1;
				v3 = object.VertexIndices[j].indices[2] + 1;

				of << "f " << v1 << "/" << v1 << "/" << v1 << " " << v2 << "/" << v2 << "/" << v2 << " " << v3 << "/" << v3 << "/" << v3 << " " << std::endl;
			}

			of.flush();
			of.close();
		}
	}

	file.close();
	return 0;
}
