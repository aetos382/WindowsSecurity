#pragma once

#include "stdafx.h"
#include "hex_manip.h"

class windows_error : public std::runtime_error
{
public:
	windows_error(
		DWORD error_code) :
		error_code(error_code),
		runtime_error(get_message(error_code))
	{
	}

	static std::string get_message(DWORD error_code)
	{
		LPSTR pszMessage = nullptr;

		DWORD result = FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr, error_code, 0, reinterpret_cast<LPSTR>(&pszMessage), 0, nullptr);

		if (result != 0)
		{
			std::ostringstream buf;
			buf << "Unknown Error" << hex(error_code) << std::endl;

			return buf.str();
		}

		std::string message = std::string(pszMessage);
		LocalFree(reinterpret_cast<HLOCAL>(pszMessage));

		return message;
	}

	static void throw_last_error()
	{
		DWORD dwLastError = GetLastError();
		if (dwLastError != ERROR_SUCCESS)
		{
			throw windows_error(dwLastError);
		}
	}

	DWORD error_code;
};
