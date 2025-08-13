#pragma once

#include "default.hpp"

enum FieldType {
    FieldType_none       = 1 << 0,
    FieldType_bool       = 1 << 1,
    FieldType_float      = 1 << 2,
    FieldType_int        = 1 << 3,
    FieldType_double     = 1 << 4,
    FieldType_Vector2    = 1 << 5,
    FieldType_Vector3    = 1 << 6,
    FieldType_Vector4    = 1 << 7,
    FieldType_Quaternion = 1 << 8,
    FieldType_GameObject = 1 << 10,
    FieldType_Transform  = 1 << 11,
    FieldType_Component  = 1 << 12,
};

class Field {
public:
    Field(const std::string &name, FieldType type, grm::size offset);

    std::string getName() const;

    FieldType getType() const;

    grm::size getOffset() const;

    template <typename T>
    static T *GetFieldData(void *ptr, const Field &field);

private:
    std::string m_name;

    FieldType m_type;

    grm::size m_offset;
};

template <typename T>
inline T *Field::GetFieldData(void *ptr, const Field &field) {
    return reinterpret_cast<T *>(reinterpret_cast<grm::uint8 *>(ptr) + field.getOffset());
}
