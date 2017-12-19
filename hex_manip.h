#pragma once

#include "stdafx.h"

struct hex_value
{
	const unsigned int value;
	const unsigned int width;
};

std::ostream & operator <<(std::ostream & out, hex_value const & bit)
{
	return out << " (0x" << std::setw(bit.width) << std::setfill('0') << std::hex << bit.value << ")";
}

hex_value hex(unsigned int value, unsigned int width)
{
	hex_value bit = { value, width };
	return bit;
}

hex_value hex(unsigned char value)
{
	return hex(value, 2);
}

hex_value hex(unsigned short value)
{
	return hex(value, 4);
}

hex_value hex(unsigned int value)
{
	return hex(value, 8);
}

hex_value hex(unsigned long value)
{
	return hex(value, 8);
}
