#pragma once

#include "stdafx.h"

class local_deleter
{
public:
	void operator()(LPVOID ptr) const
	{
		LocalFree(reinterpret_cast<HLOCAL>(ptr));
	}
};

typedef std::unique_ptr<void, local_deleter> local_ptr;
