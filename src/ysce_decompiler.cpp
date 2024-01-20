
#include "../include/yds_math.h"
#include "../include/ysce.h"

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
	std::fstream file("assets.ysce", std::ios::in | std::ios::binary);

	CompiledHeader fileHeader;
	file.read((char*)&fileHeader, sizeof(CompiledHeader));

	std::cout << "Object count: " << fileHeader.ObjectCount << std::endl;

	unsigned short* indicesFile = new unsigned short[1024 * 1024]; // 1 MB
	char* verticesFile = (char*)malloc(4 * 1024 * 1024); // 4 MB

	int currentIndexOffset = 0;
	int currentVertexByteOffset = 0;

	int initialIndex = 0;

	std::set<int> lights;
	std::map<int, int> modelIndexMap;

	for (int i = 0; i < fileHeader.ObjectCount; i++) {
		ObjectOutputHeader header;
		file.read((char*)&header, sizeof(ObjectOutputHeader));
		ObjectType objectType = static_cast<ObjectType>(header.ObjectType);

		if (objectType == ObjectType::Geometry) {
			std::cout << "New geometry object: " << header.ObjectName << std::endl;

			// create filename
			char fname[64];
			strcpy(fname, header.ObjectName);
			strcat(fname, ".obj");

			// open file
			std::fstream of(fname, std::ios::out);
			of << "# ysce-decompiler exported file" << std::endl;
			of << "# made by DDev (@ddev on Discord)" << std::endl;
			of << "# object: " << header.ObjectName << std::endl;
			of << "# vertex count: " << header.NumVertices << std::endl;
			of << "# face count: " << header.NumFaces << std::endl;
			of << "o " << header.ObjectName << std::endl;

			// read vertex size and amount of floats per vertex
			int vertexDataSize = header.VertexDataSize;
			int stride = header.VertexDataSize / header.NumVertices;

			// idk what this does
			if ((currentVertexByteOffset % stride) != 0) {
				currentVertexByteOffset += (stride - (currentVertexByteOffset % stride));
			}

			// read vertices and indices to buffer
			file.read((char*)(verticesFile + currentVertexByteOffset), vertexDataSize);
			file.read((char*)(indicesFile + currentIndexOffset), sizeof(unsigned short) * header.NumFaces * 3);

			// read vertices
			for (int j = 0; j < header.NumVertices; j++) {
				int off = j * stride;

				Convert2Float convert;
				convert.byte[0] = verticesFile[currentVertexByteOffset + off + 0];
				convert.byte[1] = verticesFile[currentVertexByteOffset + off + 1];
				convert.byte[2] = verticesFile[currentVertexByteOffset + off + 2];
				convert.byte[3] = verticesFile[currentVertexByteOffset + off + 3];
				float x = convert.real;

				convert.byte[0] = verticesFile[currentVertexByteOffset + off + 0 + 4];
				convert.byte[1] = verticesFile[currentVertexByteOffset + off + 1 + 4];
				convert.byte[2] = verticesFile[currentVertexByteOffset + off + 2 + 4];
				convert.byte[3] = verticesFile[currentVertexByteOffset + off + 3 + 4];
				float y = convert.real;

				convert.byte[0] = verticesFile[currentVertexByteOffset + off + 0 + 8];
				convert.byte[1] = verticesFile[currentVertexByteOffset + off + 1 + 8];
				convert.byte[2] = verticesFile[currentVertexByteOffset + off + 2 + 8];
				convert.byte[3] = verticesFile[currentVertexByteOffset + off + 3 + 8];
				float z = convert.real;

				// one more float here

				//std::cout << "x: " << x << " y: " << y << " z: " << z << std::endl;
				of << "v " << x << " " << y << " " << z << std::endl;

				if (header.Flags | MDF_TEXTURE_DATA) {
					convert.byte[0] = verticesFile[currentVertexByteOffset + off + 0 + 16];
					convert.byte[1] = verticesFile[currentVertexByteOffset + off + 1 + 16];
					convert.byte[2] = verticesFile[currentVertexByteOffset + off + 2 + 16];
					convert.byte[3] = verticesFile[currentVertexByteOffset + off + 3 + 16];
					float u = convert.real;

					convert.byte[0] = verticesFile[currentVertexByteOffset + off + 0 + 20];
					convert.byte[1] = verticesFile[currentVertexByteOffset + off + 1 + 20];
					convert.byte[2] = verticesFile[currentVertexByteOffset + off + 2 + 20];
					convert.byte[3] = verticesFile[currentVertexByteOffset + off + 3 + 20];
					float v = convert.real;
					of << "vt " << u << " " << v << std::endl;
				}

				if (header.Flags | MDF_NORMALS) {
					convert.byte[0] = verticesFile[currentVertexByteOffset + off + 0 + 24];
					convert.byte[1] = verticesFile[currentVertexByteOffset + off + 1 + 24];
					convert.byte[2] = verticesFile[currentVertexByteOffset + off + 2 + 24];
					convert.byte[3] = verticesFile[currentVertexByteOffset + off + 3 + 24];
					float x = convert.real;

					convert.byte[0] = verticesFile[currentVertexByteOffset + off + 0 + 28];
					convert.byte[1] = verticesFile[currentVertexByteOffset + off + 1 + 28];
					convert.byte[2] = verticesFile[currentVertexByteOffset + off + 2 + 28];
					convert.byte[3] = verticesFile[currentVertexByteOffset + off + 3 + 28];
					float y = convert.real;

					convert.byte[0] = verticesFile[currentVertexByteOffset + off + 0 + 32];
					convert.byte[1] = verticesFile[currentVertexByteOffset + off + 1 + 32];
					convert.byte[2] = verticesFile[currentVertexByteOffset + off + 2 + 32];
					convert.byte[3] = verticesFile[currentVertexByteOffset + off + 3 + 32];
					float z = convert.real;

					// one more float here

					of << "vn " << x << " " << y << " " << z << std::endl;
				}

				// skip bones
				// bytes to skip = 6
				
				// tangents don't exist in wavefront obj
				/*if (header.Flags | MDF_TANGENTS) {
					convert.byte[0] = verticesFile[currentVertexByteOffset + off + 0 + 42];
					convert.byte[1] = verticesFile[currentVertexByteOffset + off + 1 + 42];
					convert.byte[2] = verticesFile[currentVertexByteOffset + off + 2 + 42];
					convert.byte[3] = verticesFile[currentVertexByteOffset + off + 3 + 42];
					float x = convert.real;

					convert.byte[0] = verticesFile[currentVertexByteOffset + off + 0 + 46];
					convert.byte[1] = verticesFile[currentVertexByteOffset + off + 1 + 46];
					convert.byte[2] = verticesFile[currentVertexByteOffset + off + 2 + 46];
					convert.byte[3] = verticesFile[currentVertexByteOffset + off + 3 + 46];
					float y = convert.real;

					convert.byte[0] = verticesFile[currentVertexByteOffset + off + 0 + 50];
					convert.byte[1] = verticesFile[currentVertexByteOffset + off + 1 + 50];
					convert.byte[2] = verticesFile[currentVertexByteOffset + off + 2 + 50];
					convert.byte[3] = verticesFile[currentVertexByteOffset + off + 3 + 50];
					float z = convert.real;

					convert.byte[0] = verticesFile[currentVertexByteOffset + off + 0 + 54];
					convert.byte[1] = verticesFile[currentVertexByteOffset + off + 1 + 54];
					convert.byte[2] = verticesFile[currentVertexByteOffset + off + 2 + 54];
					convert.byte[3] = verticesFile[currentVertexByteOffset + off + 3 + 54];
					float w = convert.real;

					of << "vn " << x << " " << y << " " << z << " " << w << std::endl;
				}*/
			}

			// read indices
			for (int j = 0; j < header.NumFaces; j++) {
				int off = j * 3;

				// read 3 vertex indexes
				int v1 = indicesFile[currentIndexOffset + off + 0] + 1;
				int v2 = indicesFile[currentIndexOffset + off + 1] + 1;
				int v3 = indicesFile[currentIndexOffset + off + 2] + 1;

				of << "f " << v1 << "/" << v1 << "/" << v1 << " " << v2 << "/" << v2 << "/" << v2 << " " << v3 << "/" << v3 << "/" << v3 << " " << std::endl;
			}

			currentIndexOffset += header.NumFaces * 3;
			currentVertexByteOffset += header.VertexDataSize;

			of.flush();
			of.close();
		}
	}

	delete[] indicesFile;
	free(verticesFile);

	file.close();
	return 0;
}
