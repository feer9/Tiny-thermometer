#ifndef _IFLOAT32_T_H
#define _IFLOAT32_T_H

#include "utils.h"

class ifloat32_t {
private:
	int16_t integer;
	int16_t decimal;

public:
	ifloat32_t();
	ifloat32_t(int16_t _integer);
	ifloat32_t(int16_t _integer, int16_t _decimal);
	ifloat32_t(const ifloat32_t& _ifloat);		// copy constructor

	ifloat32_t& operator= (int16_t);			// integer assignment
	ifloat32_t& operator= (const ifloat32_t&);	// copy assignment

	int16_t getInteger(void) const;
	int16_t getDecimal(void) const;
	void setInteger(int16_t i);
	void setDecimal(int16_t d);
	void setValues(int16_t i, int16_t d);

	friend bool operator== (const ifloat32_t& f1, const ifloat32_t& f2);
	friend bool operator!= (const ifloat32_t& f1, const ifloat32_t& f2);
};

#endif // _IFLOAT32_T_H
