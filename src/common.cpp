
#include "../include/ysce.h"
#include "../include/dia.h"
#include "../include/yds_interchange_object.h"
#include <fstream>
#include "../include/common.h"

ysInterchangeObject readobject(std::fstream* file) {
    ObjectInformation info;
    file->read((char*)&info, sizeof(ObjectInformation));

    ysInterchangeObject object;
    object.MaterialName = info.MaterialName;
    object.Name = info.Name;
    object.ModelIndex = info.ModelIndex;
    object.ParentIndex = info.ParentIndex;
    object.InstanceIndex = info.InstanceIndex;
    switch (info.ObjectType) {
    case 0x00: object.Type = ysInterchangeObject::ObjectType::Geometry; break;
    case 0x01: object.Type = ysInterchangeObject::ObjectType::Bone; break;
    case 0x02: object.Type = ysInterchangeObject::ObjectType::Group; break;
    case 0x03: object.Type = ysInterchangeObject::ObjectType::Plane; break;
    case 0x04: object.Type = ysInterchangeObject::ObjectType::Instance; break;
    case 0x05: object.Type = ysInterchangeObject::ObjectType::Empty; break;
    case 0x06: object.Type = ysInterchangeObject::ObjectType::Armature; break;
    case 0x07: object.Type = ysInterchangeObject::ObjectType::Light; break;
    default: object.Type = ysInterchangeObject::ObjectType::Undefined; break;
    }

    const bool isInstance = object.InstanceIndex != -1;

    ObjectTransformation t;
    file->read((char*)&t, sizeof(ObjectTransformation));

    object.Orientation = t.Orientation;
    object.OrientationEuler = t.OrientationEuler;
    object.Position = t.Position;
    object.Scale = t.Scale;

    if (object.Type == ysInterchangeObject::ObjectType::Geometry) {
        GeometryInformation geometryInfo;
        file->read((char*)&geometryInfo, sizeof(GeometryInformation));

        const int normalCount = geometryInfo.NormalCount;
        const int uvChannelCount = geometryInfo.UVChannelCount;
        const int vertexCount = geometryInfo.VertexCount;
        const int faceCount = geometryInfo.FaceCount;
        const int tangentCount = geometryInfo.TangentCount;

        for (int i = 0; i < vertexCount; ++i) {
            ysVector3 v;
            file->read((char*)&v, sizeof(ysVector3));
            object.Vertices.push_back(v);
        }

        for (int i = 0; i < uvChannelCount; ++i) {
            UVChannel channel;
            file->read((char*)&channel, sizeof(UVChannel));

            ysInterchangeObject::UVChannel objectChannel;
            for (unsigned int j = 0; j < channel.UVCount; ++j) {
                ysVector2 uv;
                file->read((char*)&uv, sizeof(ysVector2));
                objectChannel.Coordinates.push_back(uv);
            }

            object.UVChannels.push_back(objectChannel);
        }

        for (int i = 0; i < normalCount; ++i) {
            ysVector3 v;
            file->read((char*)&v, sizeof(ysVector3));
            object.Normals.push_back(v);
        }

        for (int i = 0; i < tangentCount; ++i) {
            ysVector3 v;
            file->read((char*)&v, sizeof(ysVector3));
            object.Tangents.push_back(v);
        }

        for (int i = 0; i < uvChannelCount; ++i) {
            object.UVIndices.push_back({});
        }

        for (int i = 0; i < faceCount; ++i) {
            IndexSet vi, ni, ti;
            file->read((char*)&vi, sizeof(IndexSet));
            file->read((char*)&ni, sizeof(IndexSet));
            file->read((char*)&ti, sizeof(IndexSet));

            object.VertexIndices.push_back({ vi.u, vi.v, vi.w });
            object.NormalIndices.push_back({ ni.u, ni.v, ni.w });
            object.TangentIndices.push_back({ ti.u, ti.v, ti.w });

            for (int j = 0; j < uvChannelCount; ++j) {
                IndexSet u;
                file->read((char*)&u, sizeof(IndexSet));
                object.UVIndices[j].push_back({ u.u, u.v, u.w });
            }
        }
    }
    else if (object.Type == ysInterchangeObject::ObjectType::Light) {
        LightInformation lightInformation;
        file->read((char*)&lightInformation, sizeof(LightInformation));

        object.LightInformation.Color = lightInformation.Color;
        object.LightInformation.CutoffDistance = lightInformation.CutoffDistance;
        object.LightInformation.Distance = lightInformation.Distance;
        object.LightInformation.Intensity = lightInformation.Intensity;
        object.LightInformation.LightType = lightInformation.LightType;
        object.LightInformation.SpotAngularSize = lightInformation.SpotAngularSize;
        object.LightInformation.SpotFade = lightInformation.SpotFade;
    }

    return object;
}
