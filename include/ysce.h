
#pragma once

#include "yds_math.h"

struct CompiledHeader {
    int ObjectCount;
};

enum class ObjectType {
    Geometry,
    Bone,
    Group,
    Plane,
    Instance,
    Empty,
    Armature,
    Light,
    Undefined = -1
};

// Flags
static const unsigned int MDF_NONE = 0x00;
static const unsigned int MDF_BONES = 0x01;
static const unsigned int MDF_NORMALS = 0x02;
static const unsigned int MDF_TANGENTS = 0x04;
static const unsigned int MDF_TEXTURE_DATA = 0x08;
static const unsigned int MDF_ANIMATION_DATA = 0x10;

struct VertexInfo {
    bool IncludeTangents = false;
    bool IncludeNormals = true;
    bool IncludeUVs = true;

    int UVChannels = 1;
};

struct ObjectOutputHeader {
    char ObjectName[64];
    char ObjectMaterial[64];

    int ModelIndex;
    int ParentIndex;
    int ParentInstanceIndex;
    int SkeletonIndex;
    int ObjectType;

    ysVector3 Position;
    ysVector3 OrientationEuler;
    ysVector4 Orientation;
    ysVector3 Scale;

    ysVector3 MinExtreme;
    ysVector3 MaxExtreme;

    int NumUVChannels;
    int NumVertices;
    int NumFaces;

    int NumBones;
    int MaxBonesPerVertex;

    unsigned int Flags;

    int VertexDataSize;
};
