#ifndef SHMINIMAXING_SYMMETRIES_H
#define SHMINIMAXING_SYMMETRIES_H

#include <cassert>
#include <cstdint>

namespace quarto::symmetries::board
{
    inline uint16_t rotate_clk(const uint16_t mat)
    {
        return
            ((mat & 0b1000000000000000) >> 3)
            | ((mat & 0b0100000000000000) >> 6)
            | ((mat & 0b0010000000000000) >> 9)
            | ((mat & 0b0001000000000000) >> 12)

            | ((mat & 0b0000100000000000) << 2)
            | ((mat & 0b0000010000000000) >> 1)
            | ((mat & 0b0000001000000000) >> 4)
            | ((mat & 0b0000000100000000) >> 7)

            | ((mat & 0b0000000010000000) << 7)
            | ((mat & 0b0000000001000000) << 4)
            | ((mat & 0b0000000000100000) << 1)
            | ((mat & 0b0000000000010000) >> 2)

            | ((mat & 0b0000000000001000) << 12)
            | ((mat & 0b0000000000000100) << 9)
            | ((mat & 0b0000000000000010) << 6)
            | ((mat & 0b0000000000000001) << 3);
    }

    constexpr void rotate_state_clk(uint16_t* board_state)
    {
        for (int i = 0; i < 5; ++i)
        {
            board_state[i] = rotate_clk(board_state[i]);
        }
    }

    inline uint16_t mirror_vrt(const uint16_t mat)
    {
        return
            ((mat & 0x8888) >> 3) // most left
            | ((mat & 0x4444) >> 1)
            | ((mat & 0x2222) << 1)
            | ((mat & 0x1111) << 3) // most right
            ;
    }

    constexpr void mirror_state_vrt(uint16_t* board_state)
    {
        for (int i = 0; i < 5; ++i)
        {
            board_state[i] = mirror_vrt(board_state[i]);
        }
    }

    inline uint16_t mirror_hor(const uint16_t mat)
    {
        return
            ((mat & 0xf000) >> 12) // top row
            | ((mat & 0x0f00) >> 4) // 2nd row
            | ((mat & 0x00f0) << 4) // 3rd row
            | ((mat & 0x000f) << 12) // 3rd row
            ;
    }

    constexpr void mirror_state_hor(uint16_t* board_state)
    {
        for (int i = 0; i < 5; ++i)
        {
            board_state[i] = mirror_hor(board_state[i]);
        }
    }

    inline uint16_t inside_out(const uint16_t mat)
    {
        return
            ((mat & 0xa0a0) >> 5)
            | ((mat & 0x5050) >> 3)
            | ((mat & 0x0a0a) << 3)
            | ((mat & 0x0505) << 5);
    }

    constexpr void inside_state_out(uint16_t* board_state)
    {
        for (int i = 0; i < 5; ++i)
        {
            board_state[i] = inside_out(board_state[i]);
        }
    }

    inline uint16_t mid_flip(const uint16_t mat)
    {
        return
            (mat & 0x9009)
            | ((mat & 0x0900) >> 4) // right and left = swap up and down
            | ((mat & 0x0090) << 4)

            | ((mat & 0x4004) >> 1) // top and down = swap left and right
            | ((mat & 0x2002) << 1)

            | ((mat & 0x0400) >> 5)
            | ((mat & 0x0200) >> 3)
            | ((mat & 0x0040) << 3)
            | ((mat & 0x0020) << 5);
    }

    constexpr void mid_state_flip(uint16_t* board_state)
    {
        for (int i = 0; i < 5; ++i)
        {
            board_state[i] = mid_flip(board_state[i]);
        }
    }
} // board // quarto

#endif //SHMINIMAXING_SYMMETRIES_H
