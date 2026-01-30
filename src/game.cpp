#include <algorithm>
#include <cassert>
#include <iostream>
#include <ostream>

#include "search.h"
#include "game.h"

#include <limits>

#include "symmetries.h"

namespace quarto
{
    game::game(const uint16_t board_state[], uint16_t selection_state, int selected_piece)
    {
        for (int i = 0; i < 5; ++i)
        {
            this->board_state[i] = board_state[i];
        }
        this->selection_state = selection_state;
        this->selected_piece = selected_piece;

        // this->print_state();
        // std::cout << "selected piece: " << selected_piece << "selection state: " << selection_state << std::endl;
    }

    /**
     * Does a move on the board
     * @param square the square to place the selected piece to. The square must be between 0 and 15
     */
    void game::do_move(const uint8_t square)
    {
        push_state_to_undo_stack();

        assert(square < 16);
        assert(this->selected_piece != INVALID_PIECE_SELECTION);

        const uint8_t piece = QUARTO_PIECES[this->selected_piece];
        const uint16_t squareBin = 0x8000 >> square;

        for (int i = 0; i < 4; ++i)
        {
            if ((piece & 0x8 >> i) == 0x8 >> i)
            {
                this->board_state[i] |= squareBin;
            }
        }

        this->board_state[4] |= squareBin; // set that square is activated
        this->selected_piece = INVALID_PIECE_SELECTION;
    }

    void game::do_select(const uint8_t new_selection)
    {
        push_state_to_undo_stack();

        assert(new_selection < 16);
        this->selection_state &= ~(0x8000 >> new_selection);
        this->selected_piece = new_selection;
    }

    // non index based selection ONLY USE THIS ONE FOR TESTS
    void game::do_select_piece(const uint8_t piece)
    {
        assert(piece < 16);

        for (int i = 0; i < QUARTO_PIECES.size(); ++i)
        {
            if (QUARTO_PIECES[i] == piece)
            {
                do_select(i);
                return;
            }
        }
    }

    void game::push_state_to_undo_stack()
    {
        for (int i = 0; i < 5; ++i)
        {
            // put them in reverse
            previous_board_states.push(board_state[4 - i]);
        }
        previous_selected_pieces.push(selected_piece);
        previous_selection_states.push(selection_state);
    }

    void game::undo()
    {
        selected_piece = previous_selected_pieces.top();
        selection_state = previous_selection_states.top();

        for (int i = 0; i < 5; ++i)
        {
            board_state[i] = previous_board_states.top();
            previous_board_states.pop();
        }

        previous_selected_pieces.pop();
        previous_selection_states.pop();
    }

    void copy_array(const uint16_t board_state[5], uint16_t copy_to[5])
    {
        for (int i = 0; i < 5; ++i)
        {
            copy_to[i] = board_state[i];
        }
    }

    /**
     * Will try to flip all bitboards such that all bitboards have a minimal population count, if the population
     * is half of the total we will add both possiblity and iterate further on the continueing bitboards.
     *
     * @param board_states the board states you want to minimize with flips
     */
    void get_all_minimized_flips(std::vector<bb_wrapper>& board_states)
    {
        for (int bb_index = 0; bb_index < 4; ++bb_index)
        {
            int vector_size = board_states.size();

            for (int vec_index = 0; vec_index < vector_size; vec_index++)
            {
                auto& board_state_wrapper = board_states.at(vec_index);
                auto& board_state = board_state_wrapper.bb;

                if (std::popcount(board_state[bb_index]) == 8)
                {
                    // if the flip would have the same amount of 1's we need to add this possibility
                    uint16_t new_state[5];
                    copy_array(board_state, new_state);
                    board_states.push_back(create_wrapper(new_state));
                    continue;
                }

                if (std::popcount(board_state[bb_index]) > 8)
                {
                    board_state[bb_index] = ~board_state[bb_index];
                }
            }
        }
    }

    __uint128_t get_minimized_symmetrical(const std::vector<bb_wrapper>& board_states)
    {
        __uint128_t min = std::numeric_limits<__uint128_t>::max();
        for (auto game_state : board_states)
        {
            uint16_t current_state[5];
            copy_array(game_state.bb, current_state);
            uint16_t buf_state_1[5];

            for (int i = 0; i < 4; ++i)
            {
                for (int j = 0; j < i; ++j)
                {
                    symmetries::board::rotate_state_clk(current_state);
                }

                copy_array(current_state, buf_state_1);

                for (uint8_t k = 0; k < 8; ++k)
                {
                    if ((k & (0b0001)) != 0)
                    {
                        symmetries::board::mirror_state_vrt(current_state);
                    }

                    if ((k & (0b0010)) != 0)
                    {
                        symmetries::board::inside_state_out(current_state);
                    }

                    if ((k & (0b0100)) != 0)
                    {
                        symmetries::board::mid_state_flip(current_state);
                    }

                    if ((k & (0b1000)) != 0)
                    {
                        symmetries::board::inside_state_out(current_state);
                    }

                    min = std::min(min, game::format(current_state));

                    copy_array(buf_state_1, current_state);
                }
                copy_array(game_state.bb, current_state);
            }
        }
        return min;
    }

    void add_permutations(std::vector<bb_wrapper>& board_states, unsigned short game_state[5], int last_index, int i)
    {
        uint16_t new_state[5];
        copy_array(game_state, new_state);

        do
        {
            uint16_t permutated_state[5];
            copy_array(new_state, permutated_state);
            auto x = create_wrapper(permutated_state);
            board_states.push_back(x);
        }
        while (std::next_permutation(new_state + last_index, new_state + i));
    }

    /**
     * will take all game_states and reorder the attributes, this will be sorted in ascending order for the population count.
     * If there are ones with the same amount of populations then we will add all permutation of the sorting of those with the
     * same number of counts.
     * @param game_states the game_states to swap
     */
    void get_sorted_bitboard(std::vector<bb_wrapper>& board_states)
    {
        int vector_size = board_states.size();
        for (int vec_index = 0; vec_index < vector_size; vec_index++)
        {
            auto& game_state_wrapper = board_states.at(vec_index);
            auto& game_state = game_state_wrapper.bb;
            uint16_t last_num = game_state[0];
            int last_index = 0;
            std::sort(std::begin(game_state), std::end(game_state) - 1,
                      [](auto g1, auto g2) { return std::popcount(g1) > std::popcount(g2); });

            for (int i = 1; i < 4; ++i)
            {
                if (std::popcount(last_num) == std::popcount(game_state[i]))
                {
                    continue;
                }
                if (last_index != i - 1)
                {
                    add_permutations(board_states, game_state, last_index, i);
                }

                last_index = i;
            }

            if (last_index != 3)
            {
                add_permutations(board_states, game_state, last_index, 4);
            }
        }
    }

    __uint128_t game::canonize() const
    {
        uint16_t current_state[5]{};

        for (int i = 0; i < 5; ++i)
        {
            current_state[i] = this->board_state[i];
        }

        std::vector<bb_wrapper> board_states{create_wrapper(current_state)};

        get_all_minimized_flips(board_states);

        get_sorted_bitboard(board_states);

        return get_minimized_symmetrical(board_states);
    }

    bool game::is_game_over() const
    {
        return selection_state == 0 && this->board_state[4] == 0xffff;
    }

    void game::print_state() const
    {
        for (const uint16_t bs : this->board_state)
        {
            std::cout << "State: " << bs << std::endl;
        }
    }

    bool game::is_quarto() const
    {
        for (int i = 0; i < 10; ++i)
        {
            const auto quarto_magic_value = QUARTO_MAGIC_VALUES[i];

            if ((this->board_state[BOARD_PLACED] & quarto_magic_value) != quarto_magic_value)
            {
                continue;
            }


            for (int j = 0; j < 4; ++j)
            {
                if ((this->board_state[j] & quarto_magic_value) == quarto_magic_value || (~this->board_state[j] &
                    quarto_magic_value) == quarto_magic_value)
                {
                    return true;
                }
            }
        }

        return false;
    }


    uint8_t game::compute_move(int time_remaining) const
    {
        auto start = std::chrono::high_resolution_clock::now();

        search root_node = search{};
        auto result = root_node.selective_search(this->clone(), time_remaining);

        auto time_taken = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::high_resolution_clock::now() - start)
            .
            count();

        std::cout << "time taken for move: " << time_taken << std::endl;

        return result;
    }
} // quarto
