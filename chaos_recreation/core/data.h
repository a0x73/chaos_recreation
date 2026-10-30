#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <cstdint>
#include "structs.h"

class c_chaos;

class c_data
{
public:
    explicit c_data(std::atomic<bool>& application_running);
    ~c_data();

    c_data(const c_data&) = delete;
    c_data& operator=(const c_data&) = delete;

    bool begin();
    void tick();
    data_snapshot get_snapshot() const;

private:

    game_offsets offsets = game_offsets();
    
    std::unique_ptr<c_chaos> chaos = nullptr;

    data_snapshot snapshot{};
    mutable std::mutex snapshot_mutex;
    std::atomic<bool>& application_running;
    std::string process_name = "LyraGame-Win64-Shipping.exe";
};
