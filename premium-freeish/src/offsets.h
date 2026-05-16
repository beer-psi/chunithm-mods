//
// Created by beerpsi on 10/27/2024.
//

#ifndef CHUNITHM_PREMIUM_FREEISH_OFFSETS_H
#define CHUNITHM_PREMIUM_FREEISH_OFFSETS_H

#include <cstdint>
#include <cstdlib>

struct offsets_t {
    std::size_t time_date_stamp {};
    std::size_t address_of_entry_point {};

    std::uintptr_t hook_track_count {};
    std::uintptr_t hook_end_of_track {};
};

#endif //CHUNITHM_PREMIUM_FREEISH_OFFSETS_H
