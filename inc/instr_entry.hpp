#pragma once
#include <iostream>

enum InstrType {
    ARITHMETIC = 90,
    SYSCALL,
    CALL,
    JUMP,
    LDST,
    RETURN
};

struct InstrEntry {
    std::string instrDescr = "";
    InstrType type;
    // InstrEntry():instrDescr("00"), type(InstrType::RETURN){}
    // InstrEntry(std::string descr, InstrType t): instrDescr(descr), type(t) {}
};