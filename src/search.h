#ifndef SHMINIMAXING_SEARCH_TREE_H
#define SHMINIMAXING_SEARCH_TREE_H

#include <atomic>
#include <cmath>
#include <cassert>
#include <shared_mutex>
#include <unordered_map>

#include "game.h"

namespace quarto
{
    class search_node
    {
        std::atomic<int> t{0};
        std::atomic<int> n{0};
        bool visited = false;

        std::shared_ptr<search_node> parent = nullptr;
        std::vector<std::shared_ptr<search_node>> children;

    public:
        std::mutex expansion_mtx;
        uint8_t placement_move = INVALID_PIECE_SELECTION;
        uint8_t selection_move = INVALID_PIECE_SELECTION;

        search_node() = delete;

        explicit search_node(const std::shared_ptr<search_node>& node)
        {
            this->parent = node;
        }

        search_node(const search_node& other)
            : visited(other.visited),
              parent(other.parent), children(other.children),
              placement_move(other.placement_move),
              selection_move(other.selection_move)
        {
        }

        [[nodiscard]] std::shared_ptr<search_node> get_parent() const
        {
            return this->parent;
        }

        std::vector<std::shared_ptr<search_node>>& get_children()
        {
            return this->children;
        }

        [[nodiscard]] bool is_expanded() const
        {
            for (const auto& c : this->children)
            {
                if (!c->is_visited())
                {
                    return false;
                }
            }

            return true;
        }

        [[nodiscard]] bool is_visited() const
        {
            return this->visited;
        }

        void set_visited(const bool val)
        {
            this->visited = val;
        }

        [[nodiscard]] double get_t_score() const
        {
            return this->t;
        }

        void add_t_score(const int val)
        {
            this->t.fetch_add(val);
        }

        void add_visits(const int visits)
        {
            this->n.fetch_add(visits);
        }

        [[nodiscard]] int n_visits() const
        {
            return this->n;
        }

        [[nodiscard]] uint8_t format() const
        {
            return (placement_move << 4) | selection_move;
        }

        [[nodiscard]] std::shared_ptr<search_node> pick_unvisited_child() const
        {
            for (auto c : children)
            {
                if (!c->is_visited())
                {
                    return c;
                }
            }

            return nullptr;
        }

        [[nodiscard]] double get_uct(const double c_param) const
        {
            assert(parent != nullptr);

            if (n.load() == 0)
            {
                return INFINITY;
            }

            return t.load() / static_cast<double>(n.load()) + c_param * sqrt(log(parent->n_visits()) / static_cast<double>(n.load()));
        }
    };

    class search
    {
    public:
        static std::shared_ptr<search_node> best_uct(const std::shared_ptr<search_node>& node);
        [[nodiscard]] std::shared_ptr<search_node> static traverse(const std::shared_ptr<search_node>& root,
                                                                   const std::shared_ptr<game>& game_state);
        static void populate_children(const std::shared_ptr<search_node>& node,
                                      const std::shared_ptr<game>& game_state);
        [[nodiscard]] static int eval(const std::shared_ptr<game>& game_state);
        [[nodiscard]] static int rollout(const std::shared_ptr<search_node>& node,
                                         const std::shared_ptr<game>& game_state);
        static void backpropagate(const std::shared_ptr<search_node>& node, int result);
        static uint8_t best_child(const std::shared_ptr<search_node>& node);

        static uint8_t search_mnt(const std::shared_ptr<game>& game, int search_time);
        uint8_t search_dfs(const std::shared_ptr<game>& game_state);
        uint8_t selective_search(const std::shared_ptr<game>& game_state, int time_remaining);

    private:
        std::shared_mutex eval_mutex;
        void minimax_thread(uint8_t move, std::unordered_map<uint8_t, int>& eval_map, const std::shared_ptr<game>& game);
        static int max(game& game_state);
        static int max(game& game_state, int alpha, int beta, int depth);
        static int min(game& game_state, int alpha, int beta, int depth);
    };
} // quarto

#endif //SHMINIMAXING_SEARCH_TREE_H
