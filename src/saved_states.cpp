#include "saved_states.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>

saved_states* saved_states::instance = nullptr;
// https://stackoverflow.com/questions/17799134/c-singleton-undefined-reference-to

saved_states* saved_states::get_instance()
{
    if (instance == nullptr)
    {
        instance = new saved_states();
        instance->load(DEFAULT_SAVE_FILENAME);
    }
    return instance;
}

size_t saved_states::get_size() const
{
    std::shared_lock lock(this->mtx);
    return state_map.size();
}

void saved_states::store_eval(const __uint128_t state, uint8_t selection, int eval)
{
    std::unique_lock lock(this->mtx);

    if (state_map.count(state) != 1)
    {
        state_map[state] = {};
    }

    this->state_map[state][selection] = eval;
}

void saved_states::clear_zeroes()
{
    for (auto& saved_board : state_map) {
        std::erase_if(saved_board.second, [](const auto& p) { return p.second == 0; });
    }

    std::erase_if(state_map, [](const auto& p) { return p.second.empty(); });
}

int saved_states::get_value(const __uint128_t state, uint8_t selection)
{
    std::shared_lock lock(this->mtx);
    return state_map.at(state).at(selection);
}

bool saved_states::has_key(const __uint128_t state, uint8_t selection)
{
    std::shared_lock lock(this->mtx);

    if (state_map.count(state) != 1)
    {
        return false;
    }

    return state_map.at(state).count(selection) == 1;
}

void saved_states::load(std::string filename)
{
    std::ifstream file(filename + ".shmx", std::ios::in | std::ios::binary);
    if (!file)
    {
        std::cerr << "error opening state file, the saved_state table will not be populated" << std::endl;
        return;
    }

    std::vector<u_char> buffer(std::istreambuf_iterator<char>(file), {});
    deserialize(buffer);
    std::cout << "loaded: " << state_map.size() << " saved states" << std::endl;
}

void saved_states::deserialize(std::vector<u_char> data)
{
    std::cout << "started deserializing, data size: " << data.size() << std::endl;

    for (auto it = data.begin(); it != data.end();)
    {
        // 16 top bits
        uint16_t upper = static_cast<uint16_t>(*it) << 8 | static_cast<uint16_t>(*(it + 1));
        it += 2;

        // 64 lower bits
        uint64_t lower = 0;
        for (int i = 0; i < 8; ++i)
        {
            lower |= static_cast<uint64_t>(*(it + i)) << (56 - 8 * i);
        }

        it += 8;

        __uint128_t canonized = static_cast<__uint128_t>(upper) << 64 | static_cast<__uint128_t>(lower);

        // 8 bit n
        const uint8_t size = *it++;

        for (int i = 0; i < size; ++i)
        {
            const uint8_t placement = *it++;
            const int8_t eval = *it++;

            assert(eval != 0);

            state_map[canonized][placement] = eval;
        }
    }

    std::cout << "deserialized: " << state_map.size() << std::endl;
}

void saved_states::save(const std::string& filename)
{
    std::ofstream file(filename + ".shmx", std::ios::out | std::ios::binary);
    if (!file)
    {
        std::cerr << "error opening file" << std::endl;
        return;
    }

    serialize(file);

    file.close();
    std::cout << "saved state: " << state_map.size() << std::endl;
}

void saved_states::serialize(std::ofstream& file) const
{
    auto count = 0;
    for (auto v : state_map)
    {
        count += v.second.size();
    }

    std::cout << "expected save size: " << (count * 2 + 11 * state_map.size()) << " bytes" << std::endl;

    for (auto ss : state_map)
    {
        auto canonized = ss.first;
        uint16_t upper = static_cast<uint16_t>(canonized >> 64);
        uint64_t lower = static_cast<uint64_t>(canonized);
        uint8_t size = static_cast<uint8_t>(ss.second.size());

        file.write(reinterpret_cast<const char*>(&upper), sizeof(upper));
        file.write(reinterpret_cast<const char*>(&lower), sizeof(lower));
        file.write(reinterpret_cast<const char*>(&size), sizeof(size));

        for (auto sp : ss.second)
        {
            assert(sp.second != 0); // we dont want to be saving zeroes

            file.write(reinterpret_cast<const std::ostream::char_type*>(&sp.first), sizeof(sp.first));
            file.write(reinterpret_cast<const std::ostream::char_type*>(&sp.second), sizeof(sp.second));
        }
    }
}

void saved_states::clear()
{
    this->state_map.clear();
}

int saved_states::get_total_size() const
{
    auto count = 0;

    for (auto p : state_map)
    {
        count += p.second.size();
    }

    return count;
}
