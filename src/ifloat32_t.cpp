#include "ifloat32_t.h"


ifloat32_t::ifloat32_t() {
    ifloat32_t(0,0);
}

ifloat32_t::ifloat32_t(int16_t _integer) {
    ifloat32_t(_integer,0);
}

ifloat32_t::ifloat32_t(int16_t _integer, int16_t _decimal) :
    integer(_integer), decimal(_decimal) {
}

// copy constructor
ifloat32_t::ifloat32_t(const ifloat32_t& _ifloat) :
    integer(_ifloat.integer), decimal(_ifloat.decimal) {
}

// integer assignment
ifloat32_t& ifloat32_t::operator= (int16_t _integer) {
    integer = _integer;
    return *this;
}

// copy assignment
ifloat32_t& ifloat32_t::operator= (const ifloat32_t& _ifloat) {
    integer = _ifloat.integer;
    return *this;
}

int16_t ifloat32_t::getInteger(void) const {
    return integer;
}
int16_t ifloat32_t::getDecimal(void) const {
    return decimal;
}
void ifloat32_t::setInteger(int16_t i) {
    integer = i;
}
void ifloat32_t::setDecimal(int16_t d) {
    decimal = d;
}
void ifloat32_t::setValues(int16_t i, int16_t d) {
    integer = i;
    decimal = d;
}

bool operator== (const ifloat32_t &f1, const ifloat32_t &f2)
{
    return (f1.integer == f2.integer) && (f1.decimal == f2.decimal);
}

bool operator!= (const ifloat32_t &f1, const ifloat32_t &f2)
{
    return (f1.integer != f2.integer) || (f1.decimal != f2.decimal);
}
