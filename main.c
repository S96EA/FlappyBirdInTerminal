#include <stdio.h>
#include <curses.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#define FPS 30
#define FALL_SPEED 7
#define UP_SPEED 3
#define BLOCK_CHAR 'O'
#define BIRD_CHAR 'X'
#define BLOCK_WIDTH 6

typedef struct Bird {
    int position_x;
    int position_y;
} Bird;

typedef struct Block {
    int lu_x, ru_x, ld_x, rd_x;
    int lu_y, ru_y, ld_y, rd_y;
    struct Block *prev, *next;
} Block;

Bird *bird;
Block *block;
int fall_count;
int gen_block_count;
int count; // count of throw over blocks
char count_buf[16];

void fresh_block();
void fresh_bird();
void add_random_block();
void detect_collision();

void flush(int v) {
    if (fall_count == FALL_SPEED) {
        if (gen_block_count > 2 * BLOCK_WIDTH) {
            if (random() % 13 == 0) {
                gen_block_count = 0;
                add_random_block();
            }
        }
        fall_count = 0;
        fresh_bird();
        fresh_block();
        gen_block_count++;
    }
    clear();

    detect_collision();

    //show bird...
    move(bird->position_x, bird->position_y);
    addch(BIRD_CHAR);

    //show blk...
    if (block->next != block) {
        Block *tmp = block->next;
        while (tmp != block) {
            for (int row = 0; row < LINES; ++row) {
                if (row >= tmp->lu_x && row <= tmp->ld_x) {
                    continue;
                }
                for (int col = tmp->lu_y; col <= tmp->ru_y; ++col) {
                    move(row, col);
                    addch(BLOCK_CHAR);
                }
            }
            tmp = tmp->next;
        }
    }

    move(LINES-1, 0);
    sprintf(count_buf, "%d", count);
    addstr(count_buf);

    refresh();
    fall_count++;
}

void init_screen() {
    initscr();
    curs_set(0);
    crmode();
    noecho();
    keypad(stdscr, TRUE);
    clear();
}

void init_bird() {
    //init bird
    bird = malloc(sizeof(bird));
    bird->position_x = LINES / 2;
    bird->position_y = COLS / 4;
}

void fresh_bird() {
    bird->position_x++;
}

void init_block() {
    block = malloc(sizeof(Block));
    block->next = block;
    block->prev = block;
}

void add_block(Block *blk) {
    Block *next = block->next;
    block->next = blk;
    next->prev = blk;
    blk->prev = block;
    blk->next = next;
}
void add_random_block() {
    Block *blk = malloc(sizeof(Block));
    blk->lu_x = (int) (random() % (LINES * 5) / 7);
    blk->ru_x = blk->lu_x;
    blk->ld_x = blk->lu_x + (LINES * 2) / 7;
    blk->rd_x = blk->ld_x;
    blk->lu_y = COLS - 1;
    blk->ld_y = COLS - 1;
    blk->ru_y = COLS - 1;
    blk->rd_y = COLS - 1;

    add_block(blk);
}



void delete_block(Block *blk) {
    blk->prev->next = blk->next;
    blk->next->prev = blk->prev;
    free(blk);
}

void detect_collision() {
    Block *tmp = block->next;
    while (tmp != block) {
        if(bird->position_y >= tmp->lu_y && bird->position_y <= tmp->ru_y) {
            if (bird->position_x <= tmp->lu_x || bird->position_x >= tmp->ld_x) {
                move(LINES / 2, COLS / 2 - 5);
                addstr("GAME OVER!");
                ualarm(0, 0);
            }
        }
        tmp = tmp->next;
    }
}

void fresh_block() {
    Block *tmp = block->next;
    while (tmp != block) {
        if(tmp->ru_y == COLS / 4) count++; // COL / 4: position of bird
        if (tmp->ld_y != 0 && tmp->lu_y != 0) {
            tmp->ld_y--;
            tmp->lu_y--;
        }
        if (tmp->ru_y == 0 && tmp->rd_y == 0) {
            tmp = tmp->next;
            delete_block(tmp->prev);
            continue;
        }
        if (tmp->lu_y + BLOCK_WIDTH <= COLS - 1) {
            tmp->rd_y--;
            tmp->ru_y--;
        }
        tmp = tmp->next;
    }
}

void init_event() {
    //set alarm
    if (signal(SIGALRM, flush) < 0)
        exit(1);
    unsigned int interval = 1000000 / FPS;
    ualarm(interval, interval);
}

int main() {
    init_screen();
    init_bird();
    init_block();
    init_event();
    srand((unsigned int) (time(NULL) % UINT32_MAX));

    add_random_block();

    while (1) {
        int ch = getch();
        if (ch == KEY_UP || ch == 'w') { if (bird->position_x > 0) bird->position_x -= UP_SPEED; } //up
        else break;
    }
}