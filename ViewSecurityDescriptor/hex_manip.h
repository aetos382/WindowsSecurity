#pragma once

#include "stdafx.h"

struct hex_value
{
	const unsigned int width;
	const unsigned int value;
};

std::ostream & operator <<(std::ostream & out, hex_value const & bit)
{
	return out << " (0x" << std::setw(bit.width) << std::setfill('0') << std::hex << bit.value << ")";
}

hex_value hex(unsigned int width, unsigned int value)
{
	hex_value bit = { width, value };
	return bit;
}

hex_value hex(unsigned char value)
{
	return hex(2, value);
}

hex_value hex(unsigned short value)
{
	return hex(4, value);
}

hex_value hex(unsigned int value)
{
	return hex(8, value);
}

hex_value hex(unsigned long value)
{
	return hex(8, value);
}
