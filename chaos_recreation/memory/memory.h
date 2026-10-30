#pragma once
#define NOMINMAX

#include <Windows.h>
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

class memory
{
public:
    bool load(const std::string& process_name);

    std::uint64_t get_base() const noexcept { return m_base_address; }
    std::int32_t get_pid() const noexcept { return m_process_id; }

    bool read(std::uint64_t address, void* buffer, std::size_t size) const;
    bool read(std::uint64_t address, void* buffer, std::size_t size, std::int32_t pid) const;

    template <typename T>
    T read(std::uint64_t address, std::int32_t pid = 0) const
    {
        T value{};
        read(address, &value, sizeof(value), pid ? pid : m_process_id);
        return value;
    }

private:
    HANDLE process_handle_for(std::int32_t pid) const;

    std::int32_t m_process_id = 0;
    std::uint64_t m_base_address = 0;
    HANDLE m_process_handle = nullptr;
    mutable std::unordered_map<std::int32_t, HANDLE> m_process_handles;
};

inline memory mem;
