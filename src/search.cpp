#include "search.h"

#include <bitset>
#include <iostream>
#include <math.h>
#include <mutex>
#include <thread>

#include "saved_states.h"
#include "symmetries.h"

//#define DISABLE_ALPHA_BETA_SEARCH

namespace quarto
{
    std::shared_ptr<search_node> search::best_uct(const std::shared_ptr<search_node>& node)
    {
        assert(node->get_children().size() > 0);

        auto best = -INFINITY;
        std::shared_ptr<search_node> best_uct;

        for (const auto c : node->get_children())
        {
            if (const auto score = c->get_uct(1.414); best < score)
            {
                best = score;
                best_uct = c;
            }
        }

        assert(best_uct != nullptr);

        return best_uct;
    }

    std::shared_ptr<search_node> search::traverse(const std::shared_ptr<search_node>& root,
                                                  const std::shared_ptr<game>& game_state)
    {
        assert(!root->get_children().empty());
        auto picked_node = root;
        while (picked_node->is_expanded())
        {
            if (picked_node->get_children().empty()) // terminal node
            {
                return picked_node;
            }

            picked_node = best_uct(picked_node);
            assert(picked_node != root);

            assert(picked_node->placement_move < 16);
            assert(picked_node->selection_move != INVALID_PIECE_SELECTION);

            game_state->do_move(picked_node->placement_move);
            game_state->do_select(picked_node->selection_move);
        }

        std::lock_guard lock(picked_node->expansion_mtx);

        if (picked_node->get_children().empty())
        {
            populate_children(picked_node, game_state);
        }

        auto picked_child = picked_node->pick_unvisited_child();

        if (picked_child == nullptr)
        {
            return picked_node;
        }

        assert(picked_child != root);

        assert(picked_child->placement_move < 16);
        assert(picked_child->selection_move != INVALID_PIECE_SELECTION);

        game_state->do_move(picked_child->placement_move);
        game_state->do_select(picked_child->selection_move);

        assert(picked_child != nullptr);
        return picked_child;
    }

    void search::populate_children(const std::shared_ptr<search_node>& node,
                                   const std::shared_ptr<game>& game_state)
    {
        assert(node->get_children().size() == 0);
        assert(game_state->get_selection_piece() != INVALID_PIECE_SELECTION);

        if (game_state->is_game_over() || game_state->is_quarto())
        {
            return;
        }

        const auto piece_bitboards = game_state->get_board_state()[game::BOARD_PLACED];
        const auto selection_board = game_state->get_selection_state();

        for (uint8_t placement_index = 0; placement_index < 16; ++placement_index)
        {
            constexpr auto start_index = 0x8000;
            if ((piece_bitboards & (start_index >> placement_index)) != 0)
            {
                continue;
            }

            game_state->do_move(placement_index);

            for (uint8_t selection_index = 0; selection_index < 16; ++selection_index)
            {
                if ((selection_board & (start_index >> selection_index)) == 0)
                {
                    continue;
                }

                game_state->do_select(selection_index);

                auto child = std::make_shared<search_node>(search_node(node));
                child->selection_move = selection_index;
                child->placement_move = placement_index;

                node->get_children().push_back(child);

                assert(node->get_children().size() > 0);

                game_state->undo();
            }
            game_state->undo();
        }

        assert(node->get_children().size() < 241);
    }

    int search::eval(const std::shared_ptr<game>& game_state)
    {
        if (game_state->is_quarto())
        {
            if (game_state->move_side() == 1)
            {
                return -10;
            }
            return 3;
        }

        return 1;
    }

    int search::rollout(const std::shared_ptr<search_node>& node, const std::shared_ptr<game>& game_state)
    {
        node->set_visited(true);

        while (!game_state->is_game_over() && !game_state->is_quarto())
        {
            const auto piece_bitboards = game_state->get_board_state()[game::BOARD_PLACED];
            const auto selection_board = game_state->get_selection_state();

            std::vector<std::pair<uint8_t, uint8_t>> legal_moves;
            std::vector<std::pair<uint8_t, uint8_t>> winning_moves;

            for (uint8_t placement = 0; placement < 16; ++placement)
            {
                constexpr auto start_index = 0x8000;
                if ((piece_bitboards & (start_index >> placement)) != 0)
                    continue;

                for (uint8_t selection = 0; selection < 16; ++selection)
                {
                    if ((selection_board & (start_index >> selection)) == 0)
                        continue;
                    legal_moves.emplace_back(placement, selection);

                    game_state->do_move(placement);
                    game_state->do_select(selection);

                    if (game_state->is_quarto())
                    {
                        winning_moves.emplace_back(placement, selection);
                    }

                    game_state->undo();
                    game_state->undo();
                }
            }

            if (legal_moves.empty())
                break;

            if (winning_moves.empty())
            {
                auto [placement, selection] = legal_moves[std::rand() % legal_moves.size()];
                game_state->do_move(placement);
                game_state->do_select(selection);
            }
            else
            {
                auto [placement, selection] = winning_moves[std::rand() % winning_moves.size()];
                game_state->do_move(placement);
                game_state->do_select(selection);
            }
        }

        const int result = eval(game_state);
        return result;
    }

    void search::backpropagate(const std::shared_ptr<search_node>& node, const int result)
    {
        auto current = node;
        while (current)
        {
            current->add_t_score(result);
            current->add_visits(1);
            current = current->get_parent();
        }
    }

    uint8_t search::best_child(const std::shared_ptr<search_node>& node)
    {
        assert(node->get_children().size() > 0);

        int max_visits = 0;
        auto best_node = node->get_children()[0];

        for (const auto& child : node->get_children())
        {
            //std::cout << "score: " << child->get_t_score() << " visits: " << child->n_visits() << std::endl;
            if (child->n_visits() > max_visits)
            {
                max_visits = child->n_visits();
                best_node = child;
            }
        }

        // std::cout << "max visits: " << max_visits << "n of children: " << node->get_children().size() << std::endl;
        return best_node->format();
    }

    uint8_t search::search_mnt(const std::shared_ptr<game>& game_state, const int search_time)
    {
        const auto start = std::chrono::high_resolution_clock::now();
        const auto root = std::make_shared<search_node>(search_node(nullptr));
        populate_children(root, game_state);

        std::atomic<int> count = 0;

        std::vector<std::thread> search_threads;

        for (int i = 0; i < 16; ++i)
        {
            auto game_copy = game_state->clone();

            search_threads.emplace_back([search_time, game_copy, &root, start, &count]()
            {
                while (true)
                {
                    if (std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::high_resolution_clock::now() - start)
                        .
                        count() >
                        search_time)
                    {
                        break;
                    }

                    count += 1;

                    assert(!game_copy->can_undo());

                    auto leaf = traverse(root, game_copy);
                    const auto result = rollout(leaf, game_copy);
                    backpropagate(leaf, result);

                    while (game_copy->can_undo())
                    {
                        game_copy->undo();
                    }
                }
            });
        }

        for (auto& t : search_threads)
        {
            if (t.joinable())
            {
                t.join();
            }
        }

        std::cout << count << " total visits: " << root->n_visits() << "test" << std::endl;

        return best_child(root);
    }

    uint8_t search::selective_search(const std::shared_ptr<game>& game_state, const int time_remaining)
    {
        if (std::popcount(game_state->get_board_state()[game::BOARD_PLACED]) >= 7)
        {
            return search_dfs(game_state);
        }

        return search_mnt(game_state, time_remaining);
    }

    uint8_t format_move(const uint8_t placement_move, const uint8_t selection_move)
    {
        return (placement_move << 4) | selection_move;
    }

    int search::max(game& game_state)
    {
        auto init_depth = 10;
        auto pop = std::popcount(game_state.get_board_state()[4]);

        if (pop >= 7)
        {
            init_depth = 10;
        }

        return max(game_state, -1000, 1000, init_depth);
    }

    int search::max(game& game_state, int alpha, const int beta, const int depth)
    {
        const auto canonized = game_state.canonize();
        auto best_value = -1000;
        // leafnode

        if (game_state.is_quarto())
        {
            best_value = -2;
            goto EARLY_END;
        }

        if (game_state.is_game_over())
        {
            best_value = 0;
            goto EARLY_END;
        }

        if (depth == 0)
        {
            best_value = 0;
            goto EARLY_END;
        }

        if (saved_states::get_instance()->has_key(canonized, game_state.get_selection_piece()))
        {
            return saved_states::get_instance()->get_value(canonized, game_state.get_selection_piece());
        }

        for (char placement_index = 0; placement_index < 16; ++placement_index)
        {
            constexpr int start_index{0x8000};
            if ((game_state.get_board_state()[game::BOARD_PLACED] & (start_index >> placement_index)) != 0)
            {
                // if there is a piece at fieldNum [placement_index].
                continue;
            }

            game_state.do_move(placement_index);

            if (game_state.is_quarto())
            {
                best_value = 2;
                game_state.undo();
                goto EARLY_END;
            }

            if (game_state.is_game_over())
            {
                best_value = std::max(best_value, 0);
                game_state.undo();
                continue;
            }

            game_state.undo();
        }

        // non leaf node

        for (char placement_index = 0; placement_index < 16; ++placement_index)
        {
            constexpr int start_index{0x8000};
            if ((game_state.get_board_state()[game::BOARD_PLACED] & (start_index >> placement_index)) != 0)
            {
                // if there is a piece at fieldNum [placement_index].
                continue;
            }

            game_state.do_move(placement_index);

            for (char selection_index = 0; selection_index < 16; ++selection_index)
            {
                if ((game_state.get_selection_state() & (start_index >> selection_index)) == 0)
                {
                    // if there is no selection piece at index [selection_index].
                    continue;
                }

                game_state.do_select(selection_index);
                const auto score = min(game_state, alpha, beta, depth - 1);
                game_state.undo();
                if (score > best_value)
                {
                    best_value = score;
                    if (score > alpha)
                        alpha = score; // alpha acts like max in MiniMax
                }
                if (score >= beta)
                {
                    best_value = score;
                    game_state.undo();
                    goto EARLY_END;
                }
            }
            game_state.undo();
        }

    EARLY_END:

        assert(best_value != -100);
        assert(best_value != 100);

        saved_states::get_instance()->store_eval(canonized, game_state.get_selection_piece(), best_value);

        return best_value;
    }

    int search::min(game& game_state, const int alpha, int beta, const int depth)
    {
        const auto canonized = game_state.canonize();
        auto best_value = 1000;

        // leafnode

        if (game_state.is_quarto())
        {
            best_value = 2;
            goto EARLY_END;
        }

        if (game_state.is_game_over())
        {
            best_value = 0;
            goto EARLY_END;
        }

        if (depth == 0)
        {
            best_value = 0;
            goto EARLY_END;
        }

        if (saved_states::get_instance()->has_key(canonized, game_state.get_selection_piece()))
        {
            return saved_states::get_instance()->get_value(canonized, game_state.get_selection_piece());
        }

        // non leaf node

        for (char placement_index = 0; placement_index < 16; ++placement_index)
        {
            constexpr int start_index{0x8000};
            if ((game_state.get_board_state()[game::BOARD_PLACED] & (start_index >> placement_index)) != 0)
            {
                // if there is a piece at fieldNum [placement_index].
                continue;
            }

            game_state.do_move(placement_index);

            if (game_state.is_quarto())
            {
                best_value = -2;
                game_state.undo();
                goto EARLY_END;
            }

            if (game_state.is_game_over())
            {
                best_value = std::min(best_value, 0);
                game_state.undo();
                continue;
            }

            game_state.undo();
        }


        for (char placement_index = 0; placement_index < 16; ++placement_index)
        {
            constexpr int start_index{0x8000};
            if ((game_state.get_board_state()[game::BOARD_PLACED] & (start_index >> placement_index)) != 0)
            {
                // if there is a piece at fieldNum [placement_index].
                continue;
            }

            game_state.do_move(placement_index);

            for (char selection_index = 0; selection_index < 16; ++selection_index)
            {
                if ((game_state.get_selection_state() & (start_index >> selection_index)) == 0)
                {
                    // if there is no selection piece at index [selection_index].
                    continue;
                }

                game_state.do_select(selection_index);
                const auto score = max(game_state, alpha, beta, depth - 1);
                game_state.undo();
                if (score < best_value)
                {
                    best_value = score;
                    if (score < beta)
                        beta = score; // beta acts like min in MiniMax
                }
                if (score <= alpha)
                {
                    best_value = score;
                    game_state.undo();
                    goto EARLY_END;
                }
            }
            game_state.undo();
        }

    EARLY_END:

        assert(best_value != -100);
        assert(best_value != 100);

        saved_states::get_instance()->store_eval(canonized, game_state.get_selection_piece(), best_value);

        return best_value;
    }


    void search::minimax_thread(const uint8_t move, std::unordered_map<uint8_t, int>& eval_map,
                                const std::shared_ptr<game>& game)
    {
        const auto score = -max(*game);
        this->eval_mutex.lock();

        assert(eval_map.count(move) == 0);

        eval_map[move] = score;

        this->eval_mutex.unlock();
    }

    uint8_t search::search_dfs(const std::shared_ptr<game>& game_state)
    {
        const auto bitboards = game_state->get_board_state();
        const auto piece_bitboard = bitboards[game::BOARD_PLACED];
        const auto selection_board = game_state->get_selection_state();
        auto max = -1000;

        std::vector<std::thread> search_threads;
        std::unordered_map<uint8_t, int> evals;

        std::cout << "Size: " << saved_states::get_instance()->get_size() << std::endl;

        uint8_t move = 0;

        for (uint8_t placement_index = 0; placement_index < 16; ++placement_index)
        {
            constexpr int start_index{0x8000};
            if ((piece_bitboard & (start_index >> placement_index)) != 0)
            {
                // if there is a piece at fieldNum [placement_index].
                continue;
            }

            game_state->do_move(placement_index);

            if (game_state->is_quarto())
            {
                move = format_move(placement_index, 0);
                std::cout << "SHOULD WIN! Placement: " << int(placement_index) << " selected move: " << int(move) <<
                    std::endl;
                game_state->print_state();
                std::cout << "SHOULD WIN!" << std::endl;
                game_state->undo();
                goto EARLY_EXIT;
            }

            game_state->undo();
        }

        for (uint8_t placement_index = 0; placement_index < 16; ++placement_index)
        {
            constexpr int start_index{0x8000};
            if ((piece_bitboard & (start_index >> placement_index)) != 0)
            {
                continue;
            }

            game_state->do_move(placement_index);

            auto cloned = game_state->clone();

            search_threads.emplace_back([this, &evals, placement_index, cloned, selection_board]()
            {
                for (uint8_t selection_index = 0; selection_index < 16; ++selection_index)
                {
                    if ((selection_board & (start_index >> selection_index)) == 0)
                    {
                        continue;
                    }

                    cloned->do_select(selection_index);

                    minimax_thread(format_move(placement_index, selection_index), evals, cloned);

                    cloned->undo();
                }
            });

            game_state->undo();
        }

        std::cout << "all threads started: " << search_threads.size() << std::endl;

        for (auto& t : search_threads)
        {
            if (t.joinable())
            {
                t.join();
            }
        }

        for (auto ev : evals)
        {
            if (max < ev.second)
            {
                max = ev.second;
                move = ev.first;
            }
        }

        std::cout << "Max: " << max << std::endl;

    EARLY_EXIT:

        std::cout << "Size: " << saved_states::get_instance()->get_size() << std::endl;

        saved_states::get_instance()->clear_zeroes();
        // saved_states::get_instance()->save(DEFAULT_SAVE_FILENAME);

        return move;
    }
} // quarto
