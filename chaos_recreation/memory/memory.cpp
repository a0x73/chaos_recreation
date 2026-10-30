#define NOMINMAX
#include "memory.h"

#include <cstring>
#include <tlhelp32.h>
#include <utility>

namespace
{
    bool read_exact(HANDLE process, std::uint64_t address, void* buffer, std::size_t size)
    {
        if (!buffer || size == 0)
            return false;

        SIZE_T bytes_read = 0;
        if (process && address &&
            ReadProcessMemory(process, reinterpret_cast<LPCVOID>(address), buffer, size, &bytes_read) &&
            bytes_read == size)
        {
            return true;
        }

        std::memset(buffer, 0, size);
        return false;
    }

    std::wstring widen_process_name(const std::string& text)
    {
        if (text.empty() || text.size() > static_cast<std::size_t>(INT_MAX))
            return {};

        const int text_size = static_cast<int>(text.size());
        const int needed = MultiByteToWideChar(CP_ACP, 0, text.data(), text_size, nullptr, 0);
        if (needed <= 0)
            return {};

        std::wstring result(static_cast<std::size_t>(needed), L'\0');
        if (MultiByteToWideChar(CP_ACP, 0, text.data(), text_size, result.data(), needed) != needed)
            return {};
        return result;
    }

    bool get_main_module(DWORD pid, std::uint64_t& base, std::uint64_t& size)
    {
        const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
        if (snapshot == INVALID_HANDLE_VALUE)
            return false;

        MODULEENTRY32W module{};
        module.dwSize = sizeof(module);
        const bool found = Module32FirstW(snapshot, &module) != FALSE;
        if (found)
        {
            base = reinterpret_cast<std::uint64_t>(module.modBaseAddr);
            size = module.modBaseSize;
        }
        CloseHandle(snapshot);
        return found && base != 0 && size != 0;
    }
}

bool memory::load(const std::string& process_name)
{
    if (m_process_handle)
        return true;

    const std::wstring wanted = widen_process_name(process_name);
    if (wanted.empty())
        return false;

    const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
        return false;

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);

    DWORD selected_pid = 0;
    std::uint64_t selected_base = 0;
    std::uint64_t selected_size = 0;

    for (BOOL ok = Process32FirstW(snapshot, &entry); ok; ok = Process32NextW(snapshot, &entry))
    {
        if (_wcsicmp(entry.szExeFile, wanted.c_str()) != 0)
            continue;

        std::uint64_t candidate_base = 0;
        std::uint64_t candidate_size = 0;
        if (!get_main_module(entry.th32ProcessID, candidate_base, candidate_size))
            continue;

        HANDLE candidate = OpenProcess(PROCESS_VM_READ, FALSE, entry.th32ProcessID);
        if (!candidate)
            continue;

        IMAGE_DOS_HEADER dos{};
        const bool valid_image = read_exact(candidate, candidate_base, &dos, sizeof(dos)) &&
                                 dos.e_magic == IMAGE_DOS_SIGNATURE;
        CloseHandle(candidate);
        if (!valid_image)
            continue;

        selected_pid = entry.th32ProcessID;
        selected_base = candidate_base;
        selected_size = candidate_size;
        break;
    }
    CloseHandle(snapshot);

    if (!selected_pid)
        return false;

    HANDLE process = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, selected_pid);
    if (!process)
        return false;

    m_process_id = static_cast<std::int32_t>(selected_pid);
    m_base_address = selected_base;
    m_process_handle = process;
    return true;
}

HANDLE memory::process_handle_for(std::int32_t pid) const
{
    if (pid <= 0)
        return nullptr;

    if (pid == m_process_id && m_process_handle)
        return m_process_handle;

    const auto cached = m_process_handles.find(pid);
    if (cached != m_process_handles.end())
        return cached->second;

    HANDLE process = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, static_cast<DWORD>(pid));
    if (!process)
        return nullptr;

    const auto inserted = m_process_handles.emplace(pid, process);
    if (!inserted.second)
        CloseHandle(process);
    return inserted.first->second;
}

bool memory::read(std::uint64_t address, void* buffer, std::size_t size) const
{
    return read(address, buffer, size, m_process_id);
}

bool memory::read(std::uint64_t address, void* buffer, std::size_t size, std::int32_t pid) const
{
    return read_exact(process_handle_for(pid), address, buffer, size);
}
