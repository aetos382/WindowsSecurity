#include "stdafx.h"
#include "../Common/local_ptr.h"
#include "../Common/hex_manip.h"
#include "../Common/windows_error.h"

local_ptr<> GetFileSecurityDescriptor(
    LPCWSTR pszFile)
{
    PSECURITY_DESCRIPTOR pSD = nullptr;

    DWORD dwResult = GetNamedSecurityInfo(
        pszFile,
        SE_FILE_OBJECT,
        DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION,
        nullptr, nullptr, nullptr, nullptr, &pSD);

    if (dwResult != ERROR_SUCCESS)
    {
        throw windows_error(dwResult);
    }

    return local_ptr<>(pSD);
}

std::wstring GetUserName(PSID pSid)
{
    DWORD cchName = 0, cchDomain = 0;
    SID_NAME_USE sidType;

    BOOL bResult = LookupAccountSid(nullptr, pSid, nullptr, &cchName, nullptr, &cchDomain, &sidType);
    DWORD dwLastError = GetLastError();

    if (!bResult && dwLastError != ERROR_INSUFFICIENT_BUFFER)
    {
        throw windows_error(dwLastError);
    }

    auto name = std::make_unique<wchar_t[]>(cchName);
    auto domain = std::make_unique<wchar_t[]>(cchDomain);

    bResult = LookupAccountSid(nullptr, pSid, name.get(), &cchName, domain.get(), &cchDomain, &sidType);
    if (!bResult)
    {
        windows_error::throw_last_error();
    }

    if (cchDomain == 0)
    {
        return name.get();
    }
    else
    {
        std::wostringstream buf;
        buf << domain.get() << L'\\' << name.get();

        return buf.str();
    }
}

std::wstring GetOwnerName(PSECURITY_DESCRIPTOR pSD)
{
    PSID pSidOwner = nullptr;
    BOOL bOwnerDefauled;

    BOOL bResult = GetSecurityDescriptorOwner(pSD, &pSidOwner, &bOwnerDefauled);
    if (!bResult)
    {
        windows_error::throw_last_error();
    }

    auto owner_name = GetUserName(pSidOwner);
    return owner_name;
}

std::wstring GetGroupName(PSECURITY_DESCRIPTOR pSD)
{
    PSID pSidGroup = nullptr;
    BOOL bGroupDefaulted;

    BOOL bResult = GetSecurityDescriptorGroup(pSD, &pSidGroup, &bGroupDefaulted);
    if (!bResult)
    {
        windows_error::throw_last_error();
    }

    auto group_name = GetUserName(pSidGroup);
    return group_name;
}

void ShowSecurityDescriptorControlFlags(PSECURITY_DESCRIPTOR pSD)
{
    static LPCSTR control_flag_names[] = {
        "SE_OWNER_DEFAULTED",
        "SE_GROUP_DEFAULTED",
        "SE_DACL_PRESENT",
        "SE_DACL_DEFAULTED",
        "SE_SACL_PRESENT",
        "SE_SACL_DEFAULTED",
        "SE_DACL_AUTO_INHERIT_REQ",
        "SE_SACL_AUTO_INHERIT_REQ",
        "SE_DACL_AUTO_INHERITED",
        "SE_SACL_AUTO_INHERITED",
        "SE_DACL_PROTECTED",
        "SE_SACL_PROTECTED",
        "SE_RM_CONTROL_VALID",
        "SE_SELF_RELATIVE"
    };

    SECURITY_DESCRIPTOR_CONTROL control;
    DWORD revision;

    BOOL bResult = GetSecurityDescriptorControl(pSD, &control, &revision);
    if (!bResult)
    {
        windows_error::throw_last_error();
    }

    std::cout << "Control Flags:" << std::endl;

    for (int i = 0; i < _countof(control_flag_names); ++i)
    {
        USHORT bit = 1 << i;
        if (control & bit)
        {
            std::cout << "\t";

            auto right = control_flag_names[i];
            if (right)
            {
                std::wcout << right;
            }
            else
            {
                std::cout << "Unknown";
            }

            std::cout << hex(bit) << std::endl;
        }
    }
}

void ShowFileAccessRights(ACCESS_MASK access_rights)
{
    static LPCSTR access_right_names[] = {
        "FILE_READ_DATA / FILE_LIST_DIRECTORY", // 0x00000001
        "FILE_WRITE_DATA / FILE_ADD_FILE", // 0x00000002
        "FILE_APPEND_DATA / FILE_ADD_SUBDIRECTORY", // 0x00000004
        "FILE_READ_EA", // 0x00000008
        "FILE_WRITE_EA", // 0x00000010
        "FILE_EXECUTE / FILE_TRAVERSE", // 0x00000020
        "FILE_DELETE_CHILD", // 0x00000040
        "FILE_READ_ATTRIBUTES", // 0x00000080
        "FILE_WRITE_ATTRIBUTES", // 0x00000100
        nullptr, // 0x00000200
        nullptr, // 0x00000400
        nullptr, // 0x00000800
        nullptr, // 0x00001000
        nullptr, // 0x00002000
        nullptr, // 0x00004000
        nullptr, // 0x00008000
        "DELETE", // 0x00010000
        "READ_CONTROL", // 0x00020000
        "WRITE_DAC", // 0x00040000
        "WRITE_OWNER", // 0x00080000,
        "SYNCHRONIZE", // 0x00100000
    };

    std::cout << "\tRights:" << std::endl;

    for (int i = 0; i < _countof(access_right_names); ++i)
    {
        DWORD bit = 1 << i;
        if (access_rights & bit)
        {
            std::cout << "\t\t";

            auto right = access_right_names[i];
            if (right)
            {
                std::cout << right;
            }
            else
            {
                std::cout << "Unknown";
            }

            std::cout << hex(bit) << std::endl;
        }
    }
}

std::string GetAceType(BYTE ace_type)
{
    static LPCSTR ace_type_names[] = {
        "ACCESS_ALLOWED_ACE",
        "ACCESS_DENIED_ACE",
        "SYSTEM_AUDIT_ACE",
        "SYSTEM_ALARM_ACE",

        "COMPOUND_ACCESS_ALLOWED_ACE",

        "ACCESS_ALLOWED_OBJECT_ACE",
        "ACCESS_DENIED_OBJECT_ACE",
        "SYSTEM_AUDIT_OBJECT_ACE",
        "SYSTEM_ALARM_OBJECT_ACE",

        "ACCESS_ALLOWED_CALLBACK_ACE",
        "ACCESS_DENIED_CALLBACK_ACE",
        "ACCESS_ALLOWED_CALLBACK_OBJECT_ACE",
        "ACCESS_DENIED_CALLBACK_OBJECT_ACE",

        "SYSTEM_AUDIT_CALLBACK_ACE",
        "SYSTEM_ALARM_CALLBACK_ACE",
        "SYSTEM_AUDIT_CALLBACK_OBJECT_ACE",
        "SYSTEM_ALARM_CALLBACK_OBJECT_ACE"

        "SYSTEM_MANDATORY_LABEL_ACE",

        "SYSTEM_RESOURCE_ATTRIBUTE_ACE",
        "SYSTEM_SCOPED_POLICY_ID_ACE",

        "SYSTEM_PROCESS_TRUST_LABEL_ACE",
        "SYSTEM_ACCESS_FILTER_ACE"
    };

    if (ace_type > SYSTEM_ACCESS_FILTER_ACE_TYPE)
    {
        std::ostringstream buf;
        buf << "Unknown Type (" << ace_type << ")";

        return buf.str();
    }

    return ace_type_names[ace_type];
}

void ShowAceFlags(BYTE ace_flags)
{
    static LPCSTR ace_flag_names[] = {
        "OBJECT_INHERIT_ACE",
        "CONTAINER_INHERIT_ACE",
        "NO_PROPAGATE_INHERIT_ACE",
        "INHERIT_ONLY_ACE",
        "INHERITED_ACE"
    };

    std::cout << "\tFlags:" << std::endl;

    for (int i = 0; i < _countof(ace_flag_names); ++i)
    {
        unsigned char bit = 1 << i;
        if (ace_flags & bit)
        {
            std::cout << "\t\t";

            auto right = ace_flag_names[i];
            if (right)
            {
                std::cout << right;
            }
            else
            {
                std::cout << "Unknown";
            }

            std::cout << hex(bit) << std::endl;
        }
    }
}

void ShowDacl(PSECURITY_DESCRIPTOR pSD)
{
    BOOL bDaclPresent, bDaclDefauted;
    PACL pAcl = nullptr;
    BOOL bResult = GetSecurityDescriptorDacl(pSD, &bDaclPresent, &pAcl, &bDaclDefauted);

    if (!bResult)
    {
        windows_error::throw_last_error();
    }

    std::cout << "Dacl:" << std::endl;

    for (int i = 0; i < pAcl->AceCount; ++i)
    {
        void * pAce = nullptr;
        bResult = GetAce(pAcl, i, &pAce);

        if (!bResult)
        {
            windows_error::throw_last_error();
        }

        auto ace_header = static_cast<ACE_HEADER *>(pAce);
        auto ace_type = ace_header->AceType;

        auto ace_type_name = GetAceType(ace_type);

        std::cout << "\t#" << i << ": " << ace_type_name << std::endl;

        if (ace_type != ACCESS_ALLOWED_ACE_TYPE &&
            ace_type != ACCESS_DENIED_ACE_TYPE)
        {
            std::cout << "\tUnsupported" << std::endl;
            continue;
        }

        ShowAceFlags(ace_header->AceFlags);

        ACCESS_ALLOWED_ACE * pAAAce = static_cast<ACCESS_ALLOWED_ACE *>(pAce);
        PSID pSid = reinterpret_cast<PSID>(&pAAAce->SidStart);
        auto user_name = GetUserName(pSid);
        std::wcout << L"\tUser: " << user_name << std::endl;

        ShowFileAccessRights(pAAAce->Mask);

        std::cout << std::endl;
    }
}

std::wstring GetFullPath(LPCWSTR pszPath)
{
    DWORD dwResult = GetFullPathName(pszPath, 0, nullptr, nullptr);
    if (dwResult == 0)
    {
        windows_error::throw_last_error();
    }

    auto buffer = std::make_unique<wchar_t[]>(dwResult);
    dwResult = GetFullPathName(pszPath, dwResult, buffer.get(), nullptr);

    if (dwResult == 0)
    {
        windows_error::throw_last_error();
    }

    return std::wstring(buffer.get());
}

void ShowFileSecurityDescriptor(LPCWSTR pszFile)
{
    std::wcout << L"Security Descriptor of " << GetFullPath(pszFile) << std::endl;
    std::cout << std::endl;

    auto ptr = GetFileSecurityDescriptor(pszFile);
    auto pSD = static_cast<PSECURITY_DESCRIPTOR>(ptr.get());

    ShowSecurityDescriptorControlFlags(pSD);

    auto owner_name = GetOwnerName(pSD);
    std::wcout << L"Owner: " << owner_name << std::endl;

    auto group_name = GetGroupName(pSD);
    std::wcout << L"Group: " << group_name << std::endl;

    ShowDacl(pSD);
}

int wmain(
    int argc,
    wchar_t * argv[])
{
    if (argc < 2)
    {
        std::cout << "ViewSecurityDescriptor.exe <file name>" << std::endl;
        return 1;
    }

    std::wcout.imbue(std::locale(""));
    
    auto file = argv[1];

    try
    {
        ShowFileSecurityDescriptor(file);
    }
    catch (std::exception const & ex)
    {
        std::cerr << "ERROR: " << ex.what() << std::endl;
        return -1;
    }

    return 0;
}
