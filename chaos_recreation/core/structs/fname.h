#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include "../../memory/memory.h"

class f_name
{
public:

    static std::string to_string(std::uint32_t raw_index)
    {
        return resolve(raw_index, 0);
    }



private:
    static constexpr std::uint32_t pool_base_rva = 0x09370280;
    static constexpr std::uint32_t chunk_table_off = 0x10;
    static constexpr std::uint32_t entry_data_off = 0x02;
    static constexpr std::uint32_t header_len_shift = 6;
    static constexpr std::uint16_t header_wide_bit = 1;
    static constexpr std::uint32_t max_string_chars = 1024u;

    struct entry_ref_t
    {
        std::uint64_t address = 0;
        std::uint16_t header = 0;
    };


    static bool read_entry(std::uint32_t raw_index, entry_ref_t& out)
    {
        if (raw_index == 0)
            return false;

        const std::uint64_t chunk_table = mem.get_base() + pool_base_rva + chunk_table_off;
        const std::uint32_t chunk_idx = raw_index >> 16;
        const std::uint32_t slot_idx = static_cast<std::uint16_t>(raw_index);

        const auto chunk = mem.read<std::uint64_t>(
            chunk_table + static_cast<std::uint64_t>(chunk_idx) * sizeof(std::uint64_t));
        if (!chunk)
            return false;

        // IDA: chunk + 2 * LOWORD(ComparisonIndex).
        out.address = chunk + static_cast<std::uint64_t>(slot_idx) * 2u;
        out.header = mem.read<std::uint16_t>(out.address);
        return out.header != 0;
    }

    static std::uint32_t char_length(std::uint16_t header)
    {
        return static_cast<std::uint32_t>(header) >> header_len_shift;
    }

    static bool is_wide(std::uint16_t header)
    {
        return (header & header_wide_bit) != 0;
    }

    static std::string read_narrow(std::uint64_t data_addr, std::uint32_t chars)
    {
        std::string out(chars, '\0');
        if (!mem.read(data_addr, out.data(), out.size()))
            return {};
        return out;
    }

    static std::string read_wide(std::uint64_t data_addr, std::uint32_t chars)
    {
        std::vector<std::uint16_t> buffer(chars);
        if (!mem.read(data_addr, buffer.data(), buffer.size() * sizeof(std::uint16_t)))
            return {};

        std::string out(chars, '\0');
        for (std::uint32_t i = 0; i < chars; ++i)
            out[i] = static_cast<char>(buffer[i] & 0xFF);
        return out;
    }

    static std::string resolve(std::uint32_t raw_index, std::uint32_t raw_number)
    {
        (void)raw_number;

        entry_ref_t entry{};
        if (!read_entry(raw_index, entry))
            return {};

        const auto chars = char_length(entry.header);
        if (chars == 0 || chars > max_string_chars)
            return {};

        const auto data = entry.address + entry_data_off;
        return is_wide(entry.header)
            ? read_wide(data, chars)
            : read_narrow(data, chars);
    }
};

