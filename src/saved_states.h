#ifndef SHMINIMAXING_SAVED_STATES_H
#define SHMINIMAXING_SAVED_STATES_H

#include <cstdint>
#include <shared_mutex>
#include <string>
#include <unordered_map>

#include "search.h"

#define DEFAULT_SAVE_FILENAME "ss_state"

class saved_states
{
    std::unordered_map<__uint128_t, std::unordered_map<uint8_t, int8_t>> state_map;
    static saved_states* instance;
    mutable std::shared_mutex mtx;

public:
    static saved_states* get_instance();
    size_t get_size() const;

    int get_value(__uint128_t state, uint8_t selection);
    bool has_key(__uint128_t state, uint8_t selection);
    void store_eval(__uint128_t state, uint8_t selection, int eval);
    void clear_zeroes();

    void save(const std::string& filename);
    void serialize(std::ofstream& file) const;
    void deserialize(std::vector<u_char> data);
    void load(std::string filename);
    void clear();
    int get_total_size() const;
};


#endif //SHMINIMAXING_SAVED_STATES_H
