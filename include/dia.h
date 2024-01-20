
#pragma once

static constexpr int MAJOR_VERSION = 0x00;
static constexpr int MINOR_VERSION = 0x01;
static constexpr int MAGIC_NUMBER = 0xFEA4A;

struct IdHeader {
    unsigned int MagicNumber;
    unsigned int MajorVersion;
    unsigned int MinorVersion;
    unsigned int EditorId;
    unsigned int CompilationStatus;
};

struct ObjectInformation {
    char Name[256];
    char MaterialName[256];
    int ModelIndex;
    int ParentIndex;
    int InstanceIndex;
    int ObjectType;
};

struct ObjectTransformation {
    ysVector3 Position;
    ysVector3 OrientationEuler;
    ysVector4 Orientation;
    ysVector3 Scale;
};

struct LightInformation {
    int LightType;
    float Intensity;
    float CutoffDistance;
    float Distance;
    ysVector3 Color;

    float SpotAngularSize;
    float SpotFade;
};

struct SceneHeader {
    unsigned int ObjectCount;
};

struct GeometryInformation {
    unsigned int UVChannelCount;

    unsigned int VertexCount;
    unsigned int NormalCount;
    unsigned int TangentCount;
    unsigned int FaceCount;
};

struct UVChannel {
    unsigned int UVCount;
};

struct IndexSet {
    int u, v, w;
};
