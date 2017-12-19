#pragma once

#include "stdafx.h"
#include "windows_error.h"

class local_deleter
{
public:
	void operator()(LPVOID ptr) const
	{
		LocalFree(reinterpret_cast<HLOCAL>(ptr));
	}
};

template<typename T = void>
using local_ptr = std::unique_ptr<T, local_deleter>;

template<typename T = void>
local_ptr<T> make_local(size_t length)
{
	HLOCAL ptr = LocalAlloc(LPTR, length);
	if (ptr == nullptr)
	{
		windows_error::throw_last_error();
	}

	return local_ptr<T>(reinterpret_cast<typename local_ptr<T>::pointer>(ptr));
}
