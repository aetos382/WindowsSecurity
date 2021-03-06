#include "stdafx.h"
#include "../Common/local_ptr.h"
#include "../Common/windows_error.h"

local_ptr<> GetFileSecurityDescriptor(
	LPCWSTR pszFile)
{
	PSECURITY_DESCRIPTOR pSD = nullptr;

	DWORD dwResult = GetNamedSecurityInfo(
		pszFile,
		SE_FILE_OBJECT,
		DACL_SECURITY_INFORMATION,
		nullptr, nullptr, nullptr, nullptr, &pSD);

	if (dwResult != ERROR_SUCCESS)
	{
		throw windows_error(dwResult);
	}

	return local_ptr<>(pSD);
}

std::unique_ptr<SID, local_deleter> GetUserSid(LPCWSTR pszUserName)
{
	DWORD cbSid = 0;
	DWORD cchDomain = 0;
	SID_NAME_USE sidType;

	BOOL bResult = LookupAccountName(nullptr, pszUserName, nullptr, &cbSid, nullptr, &cchDomain, &sidType);
	DWORD dwError = GetLastError();

	if (!bResult && dwError != ERROR_INSUFFICIENT_BUFFER)
	{
		throw windows_error(dwError);
	}

	auto domainBuffer = std::make_unique<wchar_t[]>(cchDomain);
	auto sidBuffer = make_local<SID>(cbSid);

	bResult = LookupAccountName(nullptr, pszUserName, sidBuffer.get(), &cbSid, domainBuffer.get(), &cchDomain, &sidType);
	if (!bResult)
	{
		windows_error::throw_last_error();
	}

	return sidBuffer;
}

void AddFullAccessAllowedAce(
	PACL pAcl,
	PSID pSid)
{
	BOOL bResult = AddAccessAllowedAceEx(pAcl, ACL_REVISION, CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE, FILE_ALL_ACCESS, pSid);
	if (!bResult)
	{
		windows_error::throw_last_error();
	}
}

local_ptr<ACL> CreateNewAcl(PACL pDacl, PSID pSid)
{
	ACL_SIZE_INFORMATION aclSize = {};
	BOOL bResult = GetAclInformation(pDacl, &aclSize, sizeof(aclSize), AclSizeInformation);
	if (!bResult)
	{
		windows_error::throw_last_error();
	}

	DWORD dwNewSize = aclSize.AclBytesInUse + sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pSid) - sizeof(DWORD);
	auto newDacl = make_local<ACL>(dwNewSize);
	PACL pNewDacl = newDacl.get();

	bResult = InitializeAcl(pNewDacl, dwNewSize, ACL_REVISION);
	if (!bResult)
	{
		windows_error::throw_last_error();
	}

	return newDacl;
}

int CopyAllExplicitlyAccessDeniedAces(
	PACL pDestAcl,
	PACL pSourceAcl)
{
	int copied = 0;

	for (int i = 0; i < pSourceAcl->AceCount; ++i)
	{
		LPVOID pAce = nullptr;
		BOOL bResult = GetAce(pSourceAcl, i, &pAce);
		if (!bResult)
		{
			windows_error::throw_last_error();
		}

		auto pAceHeader = static_cast<ACE_HEADER *>(pAce);
		if (pAceHeader->AceFlags & INHERITED_ACE)
		{
			continue;
		}

		if (pAceHeader->AceType == ACCESS_DENIED_ACE_TYPE)
		{
			bResult = AddAce(pDestAcl, ACL_REVISION, MAXDWORD, pAce, pAceHeader->AceSize);
			if (!bResult)
			{
				windows_error::throw_last_error();
			}

			++copied;
		}
	}

	return copied;
}

void ChangeDacl(
	LPWSTR pszFile,
	LPCWSTR pszUserName)
{
	auto sid = GetUserSid(pszUserName);
	PSID pNewSid = sid.get();

	auto ptr = GetFileSecurityDescriptor(pszFile);
	auto pSD = static_cast<PSECURITY_DESCRIPTOR>(ptr.get());

	BOOL bDaclPresent, bDaclDefaulted;
	PACL pDacl = nullptr;
	BOOL bResult = GetSecurityDescriptorDacl(pSD, &bDaclPresent, &pDacl, &bDaclDefaulted);
	if (!bResult)
	{
		windows_error::throw_last_error();
	}

	auto newDacl = CreateNewAcl(pDacl, pNewSid);
	PACL pNewDacl = newDacl.get();

	CopyAllExplicitlyAccessDeniedAces(pNewDacl, pDacl);

	BOOL aceProcessed = FALSE;

	for (int i = 0; i < pDacl->AceCount; ++i)
	{
		LPVOID pAce = nullptr;
		bResult = GetAce(pDacl, i, &pAce);
		if (!bResult)
		{
			windows_error::throw_last_error();
		}

		auto pAceHeader = static_cast<ACE_HEADER *>(pAce);
		if (pAceHeader->AceFlags & INHERITED_ACE)
		{
			continue;
		}

		if (pAceHeader->AceType == ACCESS_DENIED_ACE_TYPE)
		{
			continue;
		}

		if (pAceHeader->AceType == ACCESS_ALLOWED_ACE_TYPE)
		{
			auto pAAAce = static_cast<ACCESS_ALLOWED_ACE *>(pAce);
			auto pSid = reinterpret_cast<PSID>(&pAAAce->SidStart);

			if (EqualSid(pSid, pNewSid))
			{
				AddFullAccessAllowedAce(pNewDacl, pNewSid);

				aceProcessed = TRUE;
				continue;
			}
		}
		
		bResult = AddAce(pNewDacl, ACL_REVISION, MAXDWORD, pAce, pAceHeader->AceSize);
		if (!bResult)
		{
			windows_error::throw_last_error();
		}
	}

	if (!aceProcessed)
	{
		AddFullAccessAllowedAce(pNewDacl, pNewSid);
	}

	DWORD dwResult = SetNamedSecurityInfo(pszFile, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr, nullptr, pNewDacl, nullptr);
	if (dwResult != ERROR_SUCCESS)
	{
		throw windows_error(dwResult);
	}
}

int wmain(
	int argc,
	wchar_t * argv[])
{
	if (argc < 3)
	{
		std::cout << "AddFullAccessAce.exe <file name> <user name>" << std::endl;
		return 1;
	}

	std::wcout.imbue(std::locale(""));

	auto file = argv[1];
	auto user = argv[2];

	try
	{
		ChangeDacl(file, user);
	}
	catch (std::exception const & ex)
	{
		std::cerr << "ERROR: " << ex.what() << std::endl;
		return -1;
	}

	return 0;
}
