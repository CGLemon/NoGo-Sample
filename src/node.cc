#include <cmath>
#include <algorithm>
#include <thread>
#include <cassert>

#include <iostream>

#include "node.h"
#include "board.h"
#include "config.h"

#define LOCK(M) \
    std::lock_guard<std::mutex> lock(m_mtx);

#define VIRTUAL_LOSS_COUNT (3)

Node::Node(int vertex) {
    m_vertex = vertex;
}

Node::Node(Node &&n) {
    m_vertex = n.m_vertex;
}

Node::~Node() {
    for (Node *n : m_children) {
        delete n;
    }
}

bool Node::expand_children(GameState &state, int &eval) {
    LOCK(m_mtx);

    if (is_expanded()) {
        return false;
    }

    int color = state.get_tomove();
    auto legal_moves = state.get_legal_moves(color);

    for (int vtx : legal_moves) {
        m_children.emplace_back(new Node(vtx));
    }

    eval = state.rollouts();
    m_expanded.store(true, std::memory_order_release);
    return true;
}

Node *Node::uct_select_child(int color) {
    wait_expanded();

    Node *best_node = nullptr;
    double best_val = std::numeric_limits<double>::lowest();

    int all_visits = 0;
    for (Node *n : m_children) {
        all_visits += n->get_visits();
    }
    all_visits = std::max(all_visits, 1);

    for (Node *n : m_children) {
        int visits = n->get_visits();
        double q = cfg_fpu_value;
        if (visits > 0) {
            q = n->get_eval(color, true);
        }
        double uct = q + cfg_c_uct *
            std::sqrt(std::log2((double)all_visits)/(visits+1));

        if (uct > best_val) {
            best_val = uct;
            best_node = n;
        }
    }
    assert(best_node != nullptr);
    return best_node;
}

int Node::get_vertex() const {
    return m_vertex;
}

int Node::get_visits() const {
    return m_visits.load(std::memory_order_relaxed);
}

std::vector<Node*> &Node::get_children() {
    return m_children;
}

int Node::get_children_size() const {
    return m_children.size();
}

double Node::get_eval(int color, bool use_virtual_loss) const {
    int visits = get_visits();
    if (use_virtual_loss) {
        visits += get_virtual_loss();
    }
    double black_eval =
        (double)m_black_wins.load(std::memory_order_relaxed)/get_visits();
    if (color == Board::WHITE) {
        return 1. - black_eval;
    }
    return black_eval;
}

void Node::update(int eval) {
    m_visits.fetch_add(1, std::memory_order_relaxed);
    m_black_wins.fetch_add(eval, std::memory_order_relaxed);
}

Node *Node::get_child(int vtx) {
    for (Node *n : m_children) {
        if (n->get_vertex() == vtx) {
            return n;
        }
    }
    return nullptr;
}

Node *Node::pop_child(int vtx) {
    LOCK(m_mtx);

    Node *n = get_child(vtx);

    if (n) {
        auto ite = std::remove_if(std::begin(m_children), std::end(m_children),
                                  [vtx](Node *a) {
                                      return a->get_vertex() == vtx;
                                  });
        m_children.erase(ite, std::end(m_children));
    }
    return n;
}

Node *Node::get_best_child() {
    wait_expanded();
    sort_children();
    return *std::begin(m_children);
}

void Node::sort_children() {
    LOCK(m_mtx);

    std::sort(std::begin(m_children), std::end(m_children),
              [](Node *a, Node *b) {
                  return a->get_visits() > b->get_visits();
              });
}

bool Node::is_expanded() const {
    return m_expanded.load(std::memory_order_acquire);
}

void Node::wait_expanded() {
    while (m_expanded.load(std::memory_order_acquire) == false) {
        std::this_thread::yield();
    }
}

int Node::count_nodes() const {
    if (get_visits() == 0) {
        return 0;
    }

    int val = 0;
    for (Node *n : m_children) {
        val += n->count_nodes();
    }
    return val+1;
}

void Node::increment_virtual_loss() {
    m_virtual_loss.fetch_add(
        VIRTUAL_LOSS_COUNT, std::memory_order_relaxed);
}

void Node::decrement_virtual_loss() {
    m_virtual_loss.fetch_sub(
        VIRTUAL_LOSS_COUNT, std::memory_order_relaxed);
}

int Node::get_virtual_loss() const {
    return m_virtual_loss.load(std::memory_order_relaxed);
}
