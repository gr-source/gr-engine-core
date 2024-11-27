#include "ecs/field.hpp"

Field::Field(const std::string &name, FieldType type, grm::size offset) : m_name(name), m_type(type), m_offset(offset) {}

std::string Field::getName() const {
    return m_name;
}

FieldType Field::getType() const {
    return m_type;
}

grm::size Field::getOffset() const {
    return m_offset;
}
