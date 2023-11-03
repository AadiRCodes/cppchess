#include <stdio.h>
#define U64 unsigned long long
#define get_bit(bb, square) (bb & (1ULL << square))
#define set_bit(bb, square) (bb |= (1ULL << square))
#define remove_bit(bb, square) (bb &= ~(1ULL << square))

// Square Constants
enum {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4, 
    a3, b3, c3, d3, e3, f3, g3, h3, 
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1
};

enum {WHITE, BLACK};
// Masks of all squares not on the a-file or h-file, not a or b-file, and not g or h-files. Latter for knights.
const U64 not_A_file = 18374403900871474942;
const U64 not_H_file = 9187201950435737471;
const U64 not_GH_file = 4557430888798830399;
const U64 not_AB_file = 18229723555195321596;

// Pawn Tables

U64 pawn_attacks[2][64];

void print_board(U64 bb) {
    // Loop over ranks (rows)
    for (int r = 0; r < 8; r++) {
        // Loop over files (columns)
        for (int f = 0; f < 8; f++) {
            int square = 8*r + f; // Squares 0-63
            if (f == 0) {
                printf(" %d  ", 8-r); // print ranks
            }
            printf(" %d ", get_bit(bb, square) ? 1 : 0); // 1ULL << square returns a mask w/ only 1 at desired square
        }
        printf("\n");
    }
    printf("     a  b  c  d  e  f  g  h \n");
    printf("\n %llu\n", bb);
}

U64 mask_pawn_attacks(int side, int square) {
    U64 attacks = 0ULL; // Bitboard containing all attacks by pawn
    U64 bitboard = 0ULL; // Bitboard of piece
    set_bit(bitboard, square);
    print_board(bitboard);
    // White
    if (side == 0) { 
        if ((bitboard) & not_H_file) attacks |= (bitboard >> 7); // Attack up + right IF ALLOWED.
        if ((bitboard) & not_A_file) attacks |= (bitboard >> 9); // Attack up + left IF ALLOWED.
    } else {
        if ((bitboard) & not_A_file) attacks |= (bitboard << 7); // Attack down + left IF ALLOWED.
        if ((bitboard) & not_H_file) attacks |= (bitboard << 9); // Attack down + right IF ALLOWED.
    }
    return attacks;
}

// Initialize pawn and knight attack maps.
void initialize_leaper_attacks() {
    for (int square = 0; square < 64; square++) {
        pawn_attacks[WHITE][square] = mask_pawn_attacks(WHITE, square);
        pawn_attacks[BLACK][square] = mask_pawn_attacks(BLACK, square);
    }
}

int main() {

    initialize_leaper_attacks();
    print_board(pawn_attacks[WHITE][h1]);
    return 0;
}