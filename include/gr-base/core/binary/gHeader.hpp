#pragma once

#include <gr-math/gmath.hpp>

#include <cstdint>
#include <cstring>

class Stream;

enum gHeaderType : int {
    gHeaderType_None     = 0,
    gHeaderType_Metadata = 1,
    gHeaderType_Texture  = 2,
    gHeaderType_Material = 3,
    gHeaderType_Mesh     = 4,
    gHeaderType_Object   = 5,
    gHeaderType_Script   = 6,
};

static const char *struct_name[7] = {
    "None",
    "Metadata",
    "Texture",
    "Material",
    "Mesh",
    "Object",
    "Script",
};

struct FileHeader {
    char signature[4];
    float version;
    size_t sizeData;
    char name[256];
};

struct DataSegment {
    gHeaderType type;
    size_t sizeData;
};

enum gStructType {
    gStructType_None     = 1 << 0,
    gStructType_Texture  = 1 << 1,
    gStructType_Material = 1 << 2,
    gStructType_Mesh     = 1 << 3,
    gStructType_Node     = 1 << 4
};

struct gVertex {
    Vector3 position;
    Vector3 normal;
    Vector2 texCoord;
    int mIdx;

    float weight[4];
    int boneIndex[4];
};

struct gBone {
    char m_name[256];

    grm::u32 m_id;

    Vector3 m_scale;

    Quaternion m_rotation;

    Vector3 m_position;
};

struct gMesh {
    char m_name[256];

    bool m_hasBone;

    gVertex *m_vertices;
    grm::u32 m_verticeCount;

    grm::u32 *m_indices;
    grm::u32 m_indicesCount;

    gBone *m_bones;
    grm::u32 m_boneCount;

    gMesh();
    ~gMesh();

    int serialize(Stream *stream, grm::size *offset = nullptr);

    int deserialize(Stream *stream);
};

enum gTextureType {
    gTextureType_albedo
};

struct gTexture {
    char m_path[256];
    gTextureType m_type;
};

struct gMaterial {
    char m_name[256];
    Color m_color;
    float m_roughness;
    float m_metallic;
    float m_ambientOcclusion;

    grm::u32 m_textureCount;
    gTexture *m_textures;
};

struct gMaterialIndex {
    char name[256];
    Color color;
    float roughness;
    float metallic;
    float ambientOcclusion;
    int index;

    grm::u32 textureCount;
    gTexture *textures;
};

struct gModel {
    char m_name[256];

    Vector3 m_scale;
    Quaternion m_rotation;
    Vector3 m_position;
};

// Animation
struct gAnimationHeader {
    char m_name[256];
    
    float m_tick;
    float m_duration;
};

struct gKeyScale {
    float time;
    Vector3 value;
};

struct gKeyRotation {
    float time;
    Quaternion value;
};

struct gKeyPosition {
    float time;
    Vector3 value;
};

struct gChannel {
    char m_name[256];

    gKeyScale *m_keyScale;
    grm::size m_keyScaleCount;

    gKeyRotation *m_keyRotation;
    grm::size m_keyRotationCount;

    gKeyPosition *m_keyPosition;
    grm::size m_keyPositionCount;
};





