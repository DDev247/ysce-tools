// ysce-decompiler.cpp : Defines the entry point for the application.
//

#include "../include/yds_math.h"
#include "../include/yds_interchange_object.h"
#include "../include/dia.h"
#include "../include/ysce.h"
#include "../include/common.h"

#include <iostream>
#include <fstream>
#include <set>
#include <map>

union Convert2Float
{
	unsigned char byte[4];
	float real;
};

union Convert2Int
{
	unsigned char byte[2];
	uint16_t real;
};

void WriteFloatToBuffer(float value, char** location) {
    *((float*)*location) = value;
    (*location) += sizeof(float);
}

void fillheader(ysInterchangeObject* object, VertexInfo info, ObjectOutputHeader* header)
{
    memset(header, 0, sizeof(ObjectOutputHeader));

    strcpy_s(header->ObjectName, 64, object->Name.c_str());
    strcpy_s(header->ObjectMaterial, 64, object->MaterialName.c_str());

    header->ModelIndex = object->ModelIndex;
    header->ParentIndex = object->ParentIndex;
    header->ParentInstanceIndex = object->InstanceIndex;
    header->SkeletonIndex = -1;
    header->ObjectType = static_cast<int>(object->Type);

    header->Position = object->Position;
    header->OrientationEuler = object->OrientationEuler;
    header->Orientation = object->Orientation;
    header->Scale = object->Scale;

    header->NumUVChannels = info.IncludeUVs ? info.UVChannels : 0;
    header->NumVertices = (int)object->Vertices.size();
    header->NumFaces = (int)object->VertexIndices.size();

    header->NumBones = 0;            // TODO
    header->MaxBonesPerVertex = 3;    // TODO

    header->Flags = MDF_NONE;

    if (header->NumBones > 0)                    header->Flags |= MDF_BONES;
    if (info.IncludeNormals)                     header->Flags |= MDF_NORMALS;
    if (info.IncludeTangents)                    header->Flags |= MDF_TANGENTS;
    if (header->NumUVChannels > 0)               header->Flags |= MDF_TEXTURE_DATA;

    float minx = FLT_MAX, miny = FLT_MAX, minz = FLT_MAX, maxx = -FLT_MAX, maxy = -FLT_MAX, maxz = -FLT_MAX;
    if (object->Type == ysInterchangeObject::ObjectType::Geometry) {
        for (int vert = 0; vert < header->NumVertices; vert++) {
            ysVector3 vertex = object->Vertices[vert];
            if (vertex.x > maxx) maxx = vertex.x;
            if (vertex.x < minx) minx = vertex.x;
            if (vertex.y > maxy) maxy = vertex.y;
            if (vertex.y < miny) miny = vertex.y;
            if (vertex.z > maxz) maxz = vertex.z;
            if (vertex.z < minz) minz = vertex.z;
        }
    }

    header->MaxExtreme = ysVector3(maxx, maxy, maxz);
    header->MinExtreme = ysVector3(minx, miny, minz);
}

int getvertexsize(ysInterchangeObject* object, const VertexInfo* info) {
    const int numUVChannels = info->IncludeUVs ? info->UVChannels : 0;
    const bool includeNormals = info->IncludeNormals;
    const bool includeTangents = info->IncludeTangents;

    int vertexSize = 4 * sizeof(float);
    /* TODO: bone weights */
    vertexSize += numUVChannels * (2 * sizeof(float));
    if (includeNormals) vertexSize += sizeof(float) * 4;
    if (includeTangents) vertexSize += sizeof(float) * 4;

    return vertexSize;
}

int packvertex(ysInterchangeObject* object, int maxBonesPerVertex, void** output, const VertexInfo* info) {
    int packedSize = 0;

    const int numUVChannels = (int)object->UVChannels.size();
    const int numRequiredUVChannels = (info->IncludeUVs) ? info->UVChannels : 0;
    const int numFaces = (int)object->VertexIndices.size();
    const int numVertices = (int)object->Vertices.size();
    const int numNormals = (int)object->Normals.size();
    const int numTangents = (int)object->Tangents.size();

    // First Calculate Size
    int vertexSize = getvertexsize(object, info);
    packedSize = vertexSize * numVertices;

    int** UVCoordinates = nullptr;
    int* normals = nullptr;
    int* tangents = nullptr;
    if (numUVChannels > 0) {
        UVCoordinates = new int* [(int)object->UVChannels.size()];
        memset(UVCoordinates, 0, sizeof(int*) * numUVChannels);

        for (int i = 0; i < numUVChannels; ++i) {
            UVCoordinates[i] = new int[numVertices];
            memset(UVCoordinates[i], 0, sizeof(int) * numVertices);
        }

        for (int face = 0; face < numFaces; ++face) {
            for (int facevert = 0; facevert < 3; ++facevert) {
                for (int channel = 0; channel < numUVChannels; ++channel) {
                    UVCoordinates[channel][object->VertexIndices[face].indices[facevert]] =
                        object->UVIndices[channel][face].indices[facevert];
                }
            }
        }
    }

    if (info->IncludeNormals && numNormals > 0) {
        normals = new int[numVertices];

        for (int face = 0; face < numFaces; ++face) {
            for (int facevert = 0; facevert < 3; ++facevert) {
                normals[object->VertexIndices[face].indices[facevert]] =
                    object->NormalIndices[face].indices[facevert];
            }
        }
    }

    if (info->IncludeTangents && numTangents > 0) {
        tangents = new int[numVertices];

        for (int face = 0; face < numFaces; ++face) {
            for (int facevert = 0; facevert < 3; ++facevert) {
                tangents[object->VertexIndices[face].indices[facevert]] =
                    object->TangentIndices[face].indices[facevert];
            }
        }
    }

    char* data = (char*)malloc(packedSize);
    char* location = data;

    for (int vert = 0; vert < numVertices; vert++) {
        ysVector3& vertex = object->Vertices[vert];

        WriteFloatToBuffer(vertex.x, &location);
        WriteFloatToBuffer(vertex.y, &location);
        WriteFloatToBuffer(vertex.z, &location);
        WriteFloatToBuffer(1.0f, &location);

        if (numRequiredUVChannels > 0) {
            for (int channel = 0; channel < numRequiredUVChannels; ++channel) {
                if (channel >= numUVChannels) {
                    WriteFloatToBuffer(0.0f, &location);
                    WriteFloatToBuffer(0.0f, &location);
                }
                else {
                    const float u = object->UVChannels[channel].Coordinates[UVCoordinates[channel][vert]].x;
                    const float v = object->UVChannels[channel].Coordinates[UVCoordinates[channel][vert]].y;

                    WriteFloatToBuffer(u, &location);
                    WriteFloatToBuffer(v, &location);
                }
            }
        }

        if (info->IncludeNormals) {
            const ysVector3& normal = (numNormals > 0)
                ? object->Normals[normals[vert]]
                : ysVector3(0.0f, 0.0f, 0.0f);

            WriteFloatToBuffer(normal.x, &location);
            WriteFloatToBuffer(normal.y, &location);
            WriteFloatToBuffer(normal.z, &location);
            WriteFloatToBuffer(0.0f, &location);
        }

        /* TODO: bones */
        if (info->IncludeTangents) {
            const ysVector3& tangent = (numTangents > 0)
                ? object->Tangents[tangents[vert]]
                : ysVector3(0.0f, 0.0f, 0.0f);

            WriteFloatToBuffer(tangent.x, &location);
            WriteFloatToBuffer(tangent.y, &location);
            WriteFloatToBuffer(tangent.z, &location);

            /* TODO: space handedness goes here */
            WriteFloatToBuffer(1.0f, &location);
        }
    }

    if (numUVChannels > 0) {
        for (int i = 0; i < numUVChannels; ++i) {
            delete[] UVCoordinates[i];
        }

        delete[] UVCoordinates;
    }

    if (numNormals > 0) delete[] normals;
    if (numTangents > 0) delete[] tangents;

    *output = data;
    return packedSize;
}

void writeobject(std::fstream* file, ysInterchangeObject* object)
{
    VertexInfo info;

    ObjectOutputHeader header;
    fillheader(object, info, &header);

    void* vertexData = nullptr;
    int vertexDataSize = 0;

    if (object->Type == ysInterchangeObject::ObjectType::Geometry) {
        vertexDataSize = packvertex(object, 4 /* TEMP */, &vertexData, &info);
        header.VertexDataSize = vertexDataSize;
    }

    file->write((char*)&header, sizeof(ObjectOutputHeader));

    // Geometry Data
    if (object->Type == ysInterchangeObject::ObjectType::Geometry) {
        file->write((char*)vertexData, vertexDataSize);

        for (size_t i = 0; i < object->VertexIndices.size(); ++i) {
            for (int facevert = 0; facevert < 3; ++facevert) {
                unsigned short reduced = object->VertexIndices[i].indices[facevert];
                file->write((char*)&reduced, sizeof(unsigned short));
            }
        }

        // TODO: Bone Map
    }
    else if (object->Type == ysInterchangeObject::ObjectType::Plane) {
        file->write((char*)&object->Length, sizeof(float));
        file->write((char*)&object->Width, sizeof(float));
    }
    else if (object->Type == ysInterchangeObject::ObjectType::Light) {
        file->write((char*)&object->LightInformation, sizeof(ysInterchangeObject::Light));
    }
}

int main(int argc, char** argv)
{
	std::cout << "Opening dia file..." << std::endl;
	std::fstream file("assets.dia", std::ios::in | std::ios::binary);

    IdHeader idHeader;
    file.read((char*)&idHeader, sizeof(IdHeader));

    if (idHeader.MagicNumber != MAGIC_NUMBER) {
        file.close();
        std::cout << "Invalid magic number" << std::endl;
        return 1;
    }

    if (idHeader.MinorVersion != MINOR_VERSION || idHeader.MajorVersion != MAJOR_VERSION) {
        file.close();
        std::cout << "Invalid version" << std::endl;
        return 1;
    }

    //m_compilationStatus = idHeader.CompilationStatus == 0x0;
    //m_toolId = idHeader.EditorId;

    SceneHeader sceneHeader;
    file.read((char*)&sceneHeader, sizeof(SceneHeader));

    std::cout << "Opening ysce file..." << std::endl;
    std::fstream ysce_file("assets.ysce", std::ios::out | std::ios::binary);
    const int objectCount = sceneHeader.ObjectCount;
    std::cout << "Object count " << objectCount << std::endl;

    CompiledHeader header;
    header.ObjectCount = objectCount;

    ysce_file.write((char*)&header, sizeof(CompiledHeader));

    for (int i = 0; i < objectCount; i++) {
        //YDS_NESTED_ERROR_CALL(toolFile.ReadObject(&objects[i]));
        // read object
        ysInterchangeObject object = readobject(&file);
        //Material* material = FindMaterial(objects[i].MaterialName.c_str());

        std::cout << "Writing object " << object.Name << std::endl;
        VertexInfo vertexInfo;
        if (object.Type == ysInterchangeObject::ObjectType::Geometry) {
            if (vertexInfo.IncludeNormals) object.RipByNormals();
            if (vertexInfo.IncludeTangents) object.RipByTangents();
            if (vertexInfo.IncludeUVs) object.RipByUVs();
        }

        object.UniformScale(1.0f);

        //exportFile.WriteObject(object);
        writeobject(&ysce_file, &object);
    }

    std::cout << "Done." << std::endl;
    ysce_file.close();
    file.close();
	return 0;
}
