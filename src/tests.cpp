#ifndef SHMINIMAXING_TESTS_H
#define SHMINIMAXING_TESTS_H

#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>

#include "game.h"
#include "saved_states.h"
#include "symmetries.h"

void test_game_init()
{
    constexpr uint16_t boardState[5]{};
    auto game = quarto::game(boardState, DEFAULT_GAME_SELECTION_STATE, 0x67);

    assert(!game.is_quarto());
    assert(!game.is_game_over());
}

void test_game_quarto()
{
    // full board
    uint16_t boardState[5] = {0x1248, 0x1248, 0x1248, 0x1248, 0x1248};
    auto game = quarto::game(boardState, 0, 0x67);
    assert(game.is_quarto());

    boardState[0] = 0b1001100000010000;
    boardState[1] = 0b1101000001000000;
    boardState[2] = 0b1101001000000000;
    boardState[3] = 0b0111000010000000;
    boardState[4] = 0b1111111111000000;

    game = quarto::game(boardState, 0, 0x67);
    assert(game.is_quarto());
}

void test_game_move()
{
    constexpr uint16_t boardState[5]{};
    auto game = quarto::game(boardState, DEFAULT_GAME_SELECTION_STATE, 0x67);
    game.do_select(0);
    game.do_move(0);
    game.do_select(0);
    assert(!game.is_quarto());
    game.do_move(1);
    game.do_select(0);
    game.do_move(2);
    game.do_select(0);
    game.do_move(3);

    assert(game.is_quarto());

    game = quarto::game(boardState, DEFAULT_GAME_SELECTION_STATE, 0x67);
    game.do_select(0);
    game.do_move(0);
    game.do_select(1);
    assert(!game.is_quarto());
    game.do_move(5);
    game.do_select(2);
    game.do_move(10);
    game.do_select(3);
    game.do_move(15);

    assert(game.is_quarto());

    game = quarto::game(boardState, DEFAULT_GAME_SELECTION_STATE, 0x67);
    game.do_select(8);
    game.do_move(0);
    game.do_select(9);
    game.do_move(4);
    assert(!game.is_quarto());
    game.do_select(10);
    game.do_move(8);
    game.do_select(11);
    game.do_move(12);

    assert(game.is_quarto());
}

void test_game_select()
{
    constexpr uint16_t boardState[5]{};
    auto game = quarto::game(boardState, DEFAULT_GAME_SELECTION_STATE, 0x67);

    game.do_select(5);
    assert(game.get_selection_piece() == 5);
    assert(game.get_selection_state() == 0xfbff);

    game.do_select(0);
    assert(game.get_selection_piece() == 0);
    assert(game.get_selection_state() == 0x7bff);
}

void test_game_undo()
{
    constexpr uint16_t boardState[5]{0, 0, 0, 0, 0};
    auto game = quarto::game(boardState, DEFAULT_GAME_SELECTION_STATE, 0x67);

    game.do_select(5);
    assert(game.get_selection_piece() == 5);
    assert(game.get_selection_state() == 0xfbff);

    game.do_select(0);
    assert(game.get_selection_piece() == 0);
    assert(game.get_selection_state() == 0x7bff);

    game.undo();
    assert(game.get_selection_piece() == 5);
    assert(game.get_selection_state() == 0xfbff);

    game.do_move(15);
    game.do_select(2);
    game.do_move(2);
    game.undo();
    assert(game.get_selection_piece() == 2);
    assert(game.get_board_state()[4] == 1);

    game.undo();
    assert(game.get_selection_piece() == 0x67);
    assert(game.get_board_state()[4] == 1);

    game.undo();
    assert(game.get_selection_piece() == 5);
    assert(game.get_board_state()[4] == 0);
}

void test_eval_pos_2_moves()
{
    constexpr uint16_t boardState[5]{};
    auto game = quarto::game(boardState, DEFAULT_GAME_SELECTION_STATE, 0x67);


    game.do_select_piece(0b0000);
    game.do_move(0);
    game.do_select_piece(0b0001);
    game.do_move(1);
    game.do_select_piece(0b1110);
    game.do_move(2);
    game.do_select_piece(0b1010);
    game.do_move(3);
    game.do_select_piece(0b1111);
    game.do_move(4);
    game.do_select_piece(0b0100);
    game.do_move(5);
    game.do_select_piece(0b1100);
    game.do_move(6);
    game.do_select_piece(0b1001);
    game.do_move(7);
    game.do_select_piece(0b0101);
    game.do_move(12);
    game.do_select_piece(0b1011);
    game.do_move(9);
    game.do_select_piece(0b0010);
    game.do_move(10);
    game.do_select_piece(0b0110);
    game.do_move(11);
    game.do_select_piece(0b1000);
    game.do_move(8);
    game.do_select_piece(0b0011);
    game.do_move(14);
    game.do_select_piece(0b0111);

    game.print_state();
    uint8_t compute_move = game.compute_move(5000);
    std::cout << int(compute_move) << " placement move: " << int(compute_move >> 4) << std::endl;
    assert((compute_move >> 4) == 15); // winning position
}

void test_eval_pos()
{
    constexpr uint16_t boardState[5]{};
    auto game = quarto::game(boardState, DEFAULT_GAME_SELECTION_STATE, 0x67);
    game.do_select_piece(0);
    game.do_move(0);
    game.do_select_piece(1);
    game.do_move(1);
    game.do_select_piece(14);
    game.do_move(2);
    game.do_select_piece(10);
    game.do_move(3);
    game.do_select_piece(15);
    game.do_move(4);
    game.do_select_piece(4);
    game.do_move(5);
    game.do_select_piece(2);
    game.do_move(10);

    game.do_select_piece(0b0101);
    game.do_move(8);

    game.do_select_piece(0b1000);
    game.do_move(12);

    game.do_select_piece(0b0100);
    game.do_move(5);

    game.do_select_piece(0b1001);
    game.do_move(7);

    game.do_select(7);
    // game.doMove(15);

    assert(!game.is_quarto());

    uint8_t compute_move = game.compute_move(5000);

    std::cout << "computed move: " << int(compute_move >> 4) << std::endl;
    assert((compute_move >> 4) == 15 || (compute_move >> 4) == 13); // winning position
}

void test_saving_loading()
{
    auto original_size = saved_states::get_instance()->get_size();
    auto original_total_size = saved_states::get_instance()->get_total_size();

    //assert(original_size != 0);

    saved_states::get_instance()->save("test");
    saved_states::get_instance()->clear();
    saved_states::get_instance()->load("test");

    assert(original_total_size == saved_states::get_instance()->get_total_size());
    assert(original_size == saved_states::get_instance()->get_size());
}

void test_long_eval()
{
    constexpr uint16_t boardState[5]{};
    auto game = quarto::game(boardState, DEFAULT_GAME_SELECTION_STATE, 0x67);

    for (int i = 0; i < 16; ++i)
    {
        game.do_select(i);
        uint8_t compute_move = game.compute_move(5000);
        game.undo();
    }

    // game.do_move(0);
    // game.do_select(1);
    // game.do_move(1);
    // game.do_select(14);
    // game.do_move(2);
    // game.do_select(10);
    // game.do_move(3);
    // game.do_select(15);
    // game.do_move(4);
    // game.do_select(4);
    //
    // uint8_t compute_move = game.compute_move(5000);
}

void test_symmetries()
{
    // mirror horizontal
    assert(quarto::symmetries::board::mirror_hor(0xf00a) == 0xa00f);
    assert(quarto::symmetries::board::mirror_hor(0xcfb0) == 0x0bfc);

    //mirror vertical
    assert(quarto::symmetries::board::mirror_vrt(0xf000) == 0xf000);
    assert(quarto::symmetries::board::mirror_vrt(0x1000) == 0x8000);
    assert(quarto::symmetries::board::mirror_vrt(0x4242) == 0x2424);

    // flip clockwise
    assert(quarto::symmetries::board::rotate_clk(0xf000) == 0x1111);
    assert(quarto::symmetries::board::rotate_clk(0x8421) == 0x1248);
    assert(quarto::symmetries::board::rotate_clk(0x124a) == 0x84a1);
    assert(quarto::symmetries::board::rotate_clk(quarto::symmetries::board::rotate_clk(0x8000)) == 1);

    // mid flip
    assert(quarto::symmetries::board::mid_flip(0x844a) == 0x822c);

    // inside out
    assert(quarto::symmetries::board::inside_out(0x9009) == 0x0660);

    // piece symmetries
    // quarto::symmetries::pieces::get_piece_symmetries();

    uint16_t boardState[5]{0xF000, 0x1000, 0x1000, 0x1000, 0xF000};
    auto game = quarto::game(boardState, DEFAULT_GAME_SELECTION_STATE, 0x67);

    uint16_t board_copy[5]{0, 0, 0, 0, 0};
    for (int i = 0; i < 5; ++i)
    {
        board_copy[i] = game.get_board_state()[i];
    }

    const auto canonized = game.canonize();

    assert(game.is_quarto());

    quarto::symmetries::board::mirror_state_hor(game.get_board_state());
    assert(game.is_quarto());
    quarto::symmetries::board::rotate_state_clk(game.get_board_state());
    assert(game.is_quarto());
    quarto::symmetries::board::mid_state_flip(game.get_board_state());
    assert(game.is_quarto());

    for (int i = 0; i < 5; ++i)
    {
        assert(board_copy[i] != game.get_board_state()[i]);
    }

    assert(canonized == game.canonize());
}

int main()
{
    std::cout << "Starting tests" << std::endl;
    test_game_init();
    std::cout << "Finished init tests" << std::endl;
    test_game_quarto();
    std::cout << "Finished quarto tests" << std::endl;
    test_game_move();
    std::cout << "Finished game move tests" << std::endl;
    test_game_select();
    std::cout << "Finished select move tests" << std::endl;
    test_game_undo();
    std::cout << "Finished undo tests" << std::endl;
    test_symmetries();
    std::cout << "Finished symetry tests" << std::endl;
    test_eval_pos();
    std::cout << "Finished searching tests" << std::endl;
    test_eval_pos_2_moves();

    //return 0;
    std::cout << "Finished searching test for 2 moves" << std::endl;
    test_saving_loading();
    std::cout << "Finished loading/saving tests" << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    test_long_eval();

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);

    std::cout << "duration: " << duration.count() << std::endl;

    test_long_eval();

    auto stop2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::seconds>(stop2 - stop);

    std::cout << "duration2: " << duration2.count() << std::endl;

    return 0;
}

#endif //SHMINIMAXING_TESTS_H
