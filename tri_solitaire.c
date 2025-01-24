/*
 * Copyright 2020 Chris Johns (cbjohns433@gmail.com)
 *
 * Written: July 8, 2020
 *
 * Solves the triangle peg game:
 *
 *               X
 *             X   X
 *           X       X
 *         X   X   X   X
 *       X   X   X   X   X
 *
 * where a move is to take a peg (X) and jump over another peg
 * to an empty space, removing the jumped-over peg.
 * So each move results in one fewer pegs on the board, and the
 * game ends when no moves are available.
 * If there is a single peg remaining, the puzzle is solved.
 * So, there are, at most, 13 moves possible, since each move
 * results in one fewer pegs on the board.
 *
 * Because triangular arrays are not really available in C, we
 * represent this as a rectangular array as follows:
 *
 *          _____________
 *          _____________
 *          ______X______
 *          _____X_X_____
 *          ____X_O_X____
 *          ___X_X_X_X___
 *          __X_X_X_X_X__
 *          _____________
 *          _____________
 *
 * where _ is an invalid space, X has a peg, and O is empty.
 * Note the borders around the figure, to allow us to access
 * cells without worrying about over-referencing the array.
 *
 * In this representation, a possible move involves a cell
 * with an X at coordinates (r,c) where the cells around it are as shown
 * and the resulting board changes follow after the arrow:
 *
 *     (r-1,c-1) == X && (r-2,c-2) == O ==> (r,c) := O, (r-1,c-1) := O, (r-2,c-2) := X
 *     (r-1,c+1) == X && (r-2,c+2) == O ==> (r,c) := O, (r-1,c+1) := O, (r-2,c+2) := X
 *     (r+1,c-1) == X && (r+2,c-2) == O ==> (r,c) := O, (r+1,c-1) := O, (r+2,c-2) := X
 *     (r+1,c+1) == X && (r+2,c+2) == O ==> (r,c) := O, (r+1,c+1) := O, (r+2,c+2) := X
 *     (r,c-2)   == X && (r,c-2)   == O ==> (r,c) := O, (r,c-2)   := O, (r,c-4)   := X
 *     (r,c+2)   == X && (r,c+2)   == O ==> (r,c) := O, (r,c+2)   := O, (r,c+4)   := X
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#undef DEBUG

#define NUM_ROWS    9
#define NUM_COLS    13
#define MAX_BOARDS  14

#define POS_EMPTY   0x00
#define POS_FULL    0x01
#define POS_INVALID 0x02
#define POS_LAST    0x10

#define O POS_EMPTY
#define X POS_FULL
#define _ POS_INVALID

#define NUM_OFFSETS 6

#define FALSE 0
#define TRUE  1

#define CLEAR_SCREEN    printf("\033[2J")
#define CURSOR_HOME     printf("\033[H")
#define INVERSE_VIDEO   printf("\033[7m")
#define NORMAL_VIDEO    printf("\033[m")

struct squares {
    int s[NUM_ROWS][NUM_COLS];
};

struct board {
    int squares[NUM_ROWS][NUM_COLS];
    struct board *next[MAX_BOARDS];
    struct board *prev;
    struct board *nextwin;
    int boardnum;
};

int rowoffsets[NUM_OFFSETS] = {-1, -1,  1, 1,  0, 0};
int coloffsets[NUM_OFFSETS] = {-1,  1, -1, 1, -2, 2};

int total_boards = 1;
int total_winning_boards = 0;
int depth = 0;
struct board *winning_board = NULL;

int debug = 0;
int visual = 0;

int generate_boards(struct board *b);

void
usage(char *name)
{
    fprintf(stderr, "usage: %s [-d] [-v]\n", name);
}

void
print_board(struct board *b)
{
    int row;
    int col;

    if (visual && !debug) {
        CURSOR_HOME;
    }

    printf("++++++++++++++++++++++\n");
    for (row = 0; row < NUM_ROWS; row++) {
        if (row >= 2 && row <= 6) {
            printf("%1d  ", row - 1);
        }
        for (col = 0; col < NUM_COLS; col++) {
            if (b->squares[row][col] & POS_FULL) {
                if (b->squares[row][col] & POS_LAST) {
                    INVERSE_VIDEO;
                }
                printf("X");
                if (b->squares[row][col] & POS_LAST) {
                    NORMAL_VIDEO;
                }
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
    printf("----------------------\n\n");
    
    if (visual && !debug) {
        sleep(1);
    }
}

int
count_pegs(struct board *b)
{
    int row;
    int col;
    int count = 0;

    for (row = 0; row < NUM_ROWS; row++) {
        for (col = 0; col < NUM_COLS; col++) {
            if (b->squares[row][col] & POS_FULL) {
                count++;
            }
        }
    }

    return count;
}

void
clear_last(struct board *b)
{
    int row;
    int col;

    for (row = 0; row < NUM_ROWS; row++) {
        for (col = 0; col < NUM_COLS; col++) {
            b->squares[row][col] &= ~POS_LAST;
        }
    }
}

int
generate_boards(struct board *b)
{
    struct board *newb;
    int row;
    int col;
    int offnum;
    int new_board_num = 0;
    int count = count_pegs(b);
    int prevnum;

    b->boardnum = total_boards;
    prevnum = (b->prev ? b->prev->boardnum : 0);

    if (debug) {
        if (count == 1) {
            printf("Board is a winner!\n");
        }
        printf("DEPTH: %d COUNT: %d BOARDNUM %d PREV %d\n", depth, count, b->boardnum, prevnum);
    }
    depth++;
    if (debug) {
        print_board(b);
    }

    if (count == 1) {
        if (winning_board == NULL) {
            winning_board = b;
        }
        total_winning_boards++;
    }

    for (row = 0; row < NUM_ROWS; row++) {
        for (col = 0; col < NUM_COLS; col++) {
            if (!(b->squares[row][col] & POS_FULL)) {
                continue;
            }

            for (offnum = 0; offnum < NUM_OFFSETS; offnum++) {
                int roff = rowoffsets[offnum];
                int coff = coloffsets[offnum];
                if ((b->squares[row + roff][col + coff] & POS_FULL) && b->squares[row + 2 * roff][col + 2 * coff] == POS_EMPTY) {
                    /* OK, we need to allocate a new board position, copy the old one and modify it */
                    newb = calloc(1, sizeof(struct board));
                    total_boards++;
                    memcpy(&newb->squares, &b->squares, sizeof(newb->squares));
                    clear_last(newb);
                    newb->squares[row][col] = POS_EMPTY;
                    newb->squares[row + roff][col + coff] = POS_EMPTY;
                    newb->squares[row + 2 * roff][col + 2 * coff] = POS_FULL | POS_LAST;

                    b->next[new_board_num] = newb;
                    newb->prev = b;
                    new_board_num++;
                    generate_boards(newb);
                }
            }
        }
    }

    depth--;

    if (new_board_num == 0) {
        return FALSE;
    } else {
        return TRUE;
    }
}

int
main(int argc, char **argv)
{
    int c;

    struct board initial_board_1 = { {
        { _, _, _, _, _, _, _, _, _, _, _, _, _ },
        { _, _, _, _, _, _, _, _, _, _, _, _, _ },
        { _, _, _, _, _, _, X, _, _, _, _, _, _ },
        { _, _, _, _, _, X, _, X, _, _, _, _, _ },
        { _, _, _, _, X, _, O, _, X, _, _, _, _ },
        { _, _, _, X, _, X, _, X, _, X, _, _, _ },
        { _, _, X, _, X, _, X, _, X, _, X, _, _ },
        { _, _, _, _, _, _, _, _, _, _, _, _, _ },
        { _, _, _, _, _, _, _, _, _, _, _, _, _ } },
        { NULL },
        NULL, 0
    };
    struct board initial_board_2 = { {
        { _, _, _, _, _, _, _, _, _, _, _, _, _ },
        { _, _, _, _, _, _, _, _, _, _, _, _, _ },
        { _, _, _, _, _, _, O, _, _, _, _, _, _ },
        { _, _, _, _, _, X, _, X, _, _, _, _, _ },
        { _, _, _, _, X, _, X, _, X, _, _, _, _ },
        { _, _, _, X, _, X, _, X, _, X, _, _, _ },
        { _, _, X, _, X, _, X, _, X, _, X, _, _ },
        { _, _, _, _, _, _, _, _, _, _, _, _, _ },
        { _, _, _, _, _, _, _, _, _, _, _, _, _ } },
        { NULL },
        NULL, 0
    };
    struct board initial_board_3 = { {
        { _, _, _, _, _, _, _, _, _, _, _, _, _ },
        { _, _, _, _, _, _, _, _, _, _, _, _, _ },
        { _, _, _, _, _, _, X, _, _, _, _, _, _ },
        { _, _, _, _, _, O, _, X, _, _, _, _, _ },
        { _, _, _, _, X, _, X, _, X, _, _, _, _ },
        { _, _, _, X, _, X, _, X, _, X, _, _, _ },
        { _, _, X, _, X, _, X, _, X, _, X, _, _ },
        { _, _, _, _, _, _, _, _, _, _, _, _, _ },
        { _, _, _, _, _, _, _, _, _, _, _, _, _ } },
        { NULL },
        NULL, 0
    };
    struct board initial_board_4 = { {
        { _, _, _, _, _, _, _, _, _, _, _, _, _ },
        { _, _, _, _, _, _, _, _, _, _, _, _, _ },
        { _, _, _, _, _, _, X, _, _, _, _, _, _ },
        { _, _, _, _, _, X, _, X, _, _, _, _, _ },
        { _, _, _, _, O, _, X, _, X, _, _, _, _ },
        { _, _, _, X, _, X, _, X, _, X, _, _, _ },
        { _, _, X, _, X, _, X, _, X, _, X, _, _ },
        { _, _, _, _, _, _, _, _, _, _, _, _, _ },
        { _, _, _, _, _, _, _, _, _, _, _, _, _ } },
        { NULL },
        NULL, 0
    };

    while ((c = getopt(argc, argv, "dv")) != EOF) {
        switch (c) {
            case 'd':
                debug = 1;
                break;
            case 'v':
                visual = 1;
                break;
            default:
                usage(argv[0]);
                exit(1);
        }
    }

    if (visual && !debug) {
        CLEAR_SCREEN;
        CURSOR_HOME;
    }

    generate_boards(&initial_board_1);

    printf("Total boards: %d\n", total_boards);

    if (total_winning_boards > 0) {
        struct board *b = winning_board;
        while (b->prev != NULL) {
            b->prev->nextwin = b;
            b = b->prev;
        }
        while (b != NULL) {
            print_board(b);
            b = b->nextwin;
        }
    }
}
