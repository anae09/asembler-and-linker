#include "section.hpp"

Section::Section(std::string name, int size) {
        this->name = name;
        this->size = size;
}

std::ostream& operator<<(std::ostream& os, struct Section& s) {
        os << s.name << '\0' << s.size;
        return os;
}