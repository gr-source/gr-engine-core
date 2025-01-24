#include "core/binary/gHeader.hpp"

#include "core/file/stream.hpp"

gMesh::gMesh() : m_hasBone(false), m_vertices(nullptr), m_verticeCount(0), m_indices(nullptr), m_indicesCount(0), m_bones(nullptr), m_boneCount(0) {}

gMesh::~gMesh() {
    if (m_vertices) {
        delete []m_vertices;
    }
    if (m_indices) {
        delete []m_indices;
    }
    if (m_bones && m_hasBone) {
        delete []m_bones;
    }
}

int gMesh::serialize(Stream *stream, grm::size *offset) {
    if (offset) {
        *offset += sizeof(char)     * 256;
        *offset += sizeof(bool);
        *offset += sizeof(gVertex)  * m_verticeCount;
        *offset += sizeof(grm::u32) * m_indicesCount;
        *offset += sizeof(gBone)    * m_boneCount;
    }

    // metadata
    if (!stream->writeS(m_name, 256, false)) {
        return 1;
    }
    if (!stream->writeS(&m_hasBone, 1, false)) {
        return 2;
    }

    // data
    if (!stream->writeS(&m_verticeCount, 1, false)) {
        return 3;
    }
    if (!stream->writeS(m_vertices, m_verticeCount, false)) {
        return 4;
    }
    
    if (!stream->writeS(&m_indicesCount, 1, false)) {
        return 5;
    }
    if (!stream->writeS(m_indices, m_indicesCount, false)) {
        return 6;
    }

    if (m_hasBone) {
        if (!stream->writeS(&m_boneCount, 1, false)) {
            return 7;
        }
        if (!stream->writeS(m_bones, m_boneCount, false)) {
            return 8;
        }
    }
    return 0;
}

int gMesh::deserialize(Stream *stream) {
    // metadata
    if (!stream->readS(m_name, 256)) {
        return 1;
    }
    if (!stream->readS(&m_hasBone, 1)) {
        return 2;
    }

    // data
    // vertice
    {
        if (!stream->readS(&m_verticeCount, 1)) {
            return 3;
        }
        if (m_vertices) {
            delete []m_vertices;
        }
        m_vertices = new gVertex[m_verticeCount];
        
        if (!stream->readS(m_vertices, m_verticeCount)) {
            return 4;
        }
    }
    
    // indices
    {
        if (!stream->readS(&m_indicesCount, 1)) {
            return 5;
        }
        if (m_indices) {
            delete []m_indices;
        }
        m_indices = new grm::u32[m_indicesCount];
        if (!stream->readS(m_indices, m_indicesCount)) {
            return 6;
        }
    }

    // bone
    if (m_hasBone) {
        if (!stream->readS(&m_boneCount, 1)) {
            return 7;
        }
        if (m_bones) {
            delete []m_bones;
        }
        m_bones = new gBone[m_boneCount];
        if (!stream->readS(m_bones, m_boneCount)) {
            return 8;
        }
    }
    return 0;
}
