#ifndef GAMESTATE_H_INCLUDE
#define GAMESTATE_H_INCLUDE

#include <vector>
#include <memory>
#include <iostream>

#include "board.h"

class GameState {
public:
    // Clear the board.
    void clear_board(int board_size, float komi);

    // Reture all legal moves.
    std::vector<int> get_legal_moves(int color) const;

    // Reture true if there is no legal move.
    bool is_gameover(int color) const;

    // Return true if the move is legal and play it.
    bool play_move(int vtx, int color);

    //
    bool play_move_fast(int vtx, int color);

    // Generate a random move and play it.
    int play_random_move(int color, bool use_fast=false);

    //
    int rollouts();

    // Return true if the move is legal.
    bool legal_move(int vtx, int color);

    // Return true if the current board is superko.
    bool superko();

    // Undo the current move.
    void undo_move();

    // Compute the final score with Tromp-Taylor rule. Return
    // the black score.
    float final_score();

    // Show the currnet board.
    void showboard();

    // Transfer the vertex to x coordinate.
    int get_x(int vtx) const;

    // Transfer the vertex to y coordinate.
    int get_y(int vtx) const;

    // Get the vertex position.
    int get_vertex(int x, int y) const;

    // Get the index position.
    int get_index(int x, int y) const;

    // Get the side to move.
    int get_tomove() const;

    // Get the last move.
    int get_last_move() const;

    // Get the ko move of current board.
    int get_komove() const;

    // Get the board size.
    int get_board_size() const;

    // Get the currnet passes number.
    int get_passes() const;

    // Get the game komi.
    float get_komi() const;

    int get_movenum() const;

    // Get the current board state(black/white/empty) by the vertex 
    // position.
    int get_state(int vtx) const;

    // Set the game komi.
    void set_komi(float komi);

    // Set the side to move color.
    void set_to_move(int color);

    // The current board.
    Board board;

private:
    std::vector<std::shared_ptr<const Board>> m_game_history;

    float m_komi;

    int m_movenum;
};

#endif
