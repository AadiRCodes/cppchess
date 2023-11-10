#include <stdio.h>
#define U64 unsigned long long
#define get_bit(bb, square) (bb & (1ULL << square))
#define set_bit(bb, square) (bb |= (1ULL << square))
#define remove_bit(bb, square) (bb &= ~(1ULL << square))
#define count_bits(bb) __builtin_popcountll(bb)
#define first_bit(bb) __builtin_ffsll(bb)-1



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

// Attack Maps

U64 pawn_attacks[2][64];
U64 knight_attacks[64];
U64 king_attacks[64];

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

/* ********* LEAPER (UNBLOCKABLE) PIECE ATTACKS ********* */

/* Initialize Attack Map for Square */
U64 mask_pawn_attacks(int side, int square) {
    U64 attacks = 0ULL; // Bitboard containing all attacks by pawn
    U64 bitboard = 0ULL; // Bitboard of piece
    set_bit(bitboard, square);
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

U64 mask_knight_attacks(int square) {
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;
    set_bit(bitboard, square);
    if ((bitboard) & not_A_file) {
        attacks |= (bitboard >> 17); // Attack up 2, left 1
        attacks |= (bitboard << 15); // Attack down 2, left 1
    }  // Attack up 2, left 1
    if ((bitboard) & not_H_file) {
        attacks |= (bitboard >> 15); // Attack up 2, right 1
        attacks |= (bitboard << 17); // Attack down 2, right 1
    } // Attack up 2, right 1
    if ((bitboard) & not_AB_file) {
        attacks |= (bitboard >> 10); // Attack up 1, left 2
        attacks |= (bitboard << 6); // Attack down 1, left 2
    }
    if ((bitboard) & not_GH_file) {
        attacks |= (bitboard >> 6); // Attack up 1, right 2
        attacks |= (bitboard << 10); // Attack down 1, right 2
    }
    
    return attacks;
}

U64 mask_king_attacks(int square) {
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;
    set_bit(bitboard, square);
    attacks |= (bitboard << 8);
    attacks |= (bitboard >> 8);
    if (bitboard & not_A_file) {
        attacks |= (bitboard >> 9);
        attacks |= (bitboard >> 1);
        attacks |= (bitboard << 7);
    }
    if (bitboard & not_H_file) {
        attacks |= (bitboard << 9);
        attacks |= (bitboard << 1);
        attacks |= (bitboard >> 7);
    }
    return attacks;
}

// Initialize pawn, knight, and king attack maps.
void initialize_leaper_attacks() {
    for (int square = 0; square < 64; square++) {
        pawn_attacks[WHITE][square] = mask_pawn_attacks(WHITE, square);
        pawn_attacks[BLACK][square] = mask_pawn_attacks(BLACK, square);
        knight_attacks[square] = mask_knight_attacks(square);
        king_attacks[square] = mask_king_attacks(square);
    }
}

/* ********* SLIDING PIECE ATTACKS ********** */
// A precalculated table of the number of bits in the occupancy mask of a bishop on each square.
const int bishop_occupancy_bit_count[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};
const int rook_occupancy_bit_count[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
 };

/* Why don't we take the edge of the board? Well, OCCUPANCY bits are bits in which another piece could BLOCK
 A BISHOP or ROOK which will AFFECT the attack squares of the sliding pieces. Since a piece on the edge of the board
 does not impact any squares (as there are none behind them) we ignore them in this computation. */

U64 mask_bishop_occupancies(int square) {
    // Returns a mask with a 1 in every square that can affect the bishop's attack range, known as its occupancy mask.
    U64 attacks = 0ULL;
    int r, f;
    int target_rank = square/8;
    int target_file = square%8;

    // Mask bishop occupancy bits
    for (r = target_rank+1, f = target_file+1; r <= 6 && f <= 6; r++, f++) {
        set_bit(attacks, (8*r + f));
    }
    for (r = target_rank-1, f = target_file+1; r >= 1 && f <= 6; r--, f++) {
        set_bit(attacks, (8*r + f));
    }
    for (r = target_rank+1, f = target_file-1; r <= 6 && f >= 1; r++, f--) {
        set_bit(attacks, (8*r + f));
    }
    for (r = target_rank-1, f = target_file-1; r >= 1 && f >= 1; r--, f--) {
        set_bit(attacks, (8*r + f));
    }
    return attacks;
}
U64 mask_rook_occupancies(int square) {
// Returns a mask with a 1 in every square that can affect the bishop's attack range, known as its occupancy mask.

    // INPUT: Square of piece

    U64 attacks = 0ULL;
    int r, f;
    int target_rank = square/8;
    int target_file = square%8;

    // Mask rook occupancy bits
    for (r = target_rank+1; r <= 6; r++) {
        set_bit(attacks, (8*r + target_file));
    }
    for (r = target_rank-1; r >= 1; r--) {
        set_bit(attacks, (8*r + target_file));
    }
    for (f = target_file-1; f >= 1; f--) {
        set_bit(attacks, (8*target_rank + f));
    }
    for (f = target_file+1; f <= 6; f++) {
        set_bit(attacks, (8*target_rank + f));
    }
    return attacks;
}

U64 bishop_attacks_imm(int square, U64 blocked) {
    // This is a helper function used to generate sliding piece attacks. These do not actually 
    // generate attacks, but the result is used in an algorithm known as "magic bitboards" that will
    // actually help generate the attacks.
    // INPUT: square of bishop, bitboard of all blocking pieces
    U64 attacks = 0ULL;
    int r, f;
    int target_rank = square/8;
    int target_file = square%8;

    // Mask bishop occupancy bits
    for (r = target_rank+1, f = target_file+1; r <= 7 && f <= 7; r++, f++) {
        set_bit(attacks, (8*r + f));
        if (get_bit(blocked, (8*r+f))) {
            break;
        }
    }
    for (r = target_rank-1, f = target_file+1; r >= 0 && f <= 7; r--, f++) {
        set_bit(attacks, (8*r + f));
        if (get_bit(blocked, (8*r+f))) {
            break;
        }
    }
    for (r = target_rank+1, f = target_file-1; r <= 7 && f >= 0; r++, f--) {
        set_bit(attacks, (8*r + f));
        if (get_bit(blocked, (8*r+f))) {
            break;
        }
    }
    for (r = target_rank-1, f = target_file-1; r >= 0 && f >= 0; r--, f--) {
        set_bit(attacks, (8*r + f));
        if (get_bit(blocked, (8*r+f))) {
            break;
        }
    }
    return attacks;
}

U64 rook_attacks_imm(int square, U64 blocked) {
    // This is a helper function used to generate sliding piece attacks. These do not actually 
    // generate attacks, but the result is used in an algorithm known as "magic bitboards" that will
    // actually help generate the attacks.

    // INPUT: Square of piece, bitboard of blocking pieces

    U64 attacks = 0ULL;
    int r, f;
    int target_rank = square/8;
    int target_file = square%8;

    // Mask rook occupancy bits
    for (r = target_rank+1; r <= 7; r++) {
        set_bit(attacks, (8*r + target_file));
        if (get_bit(blocked, (8*r+target_file))) {
            break;
        }
    }
    for (r = target_rank-1; r >= 0; r--) {
        set_bit(attacks, (8*r + target_file));
        if (get_bit(blocked, (8*r+target_file))) {
            break;
        }
    }
    for (f = target_file-1; f >= 0; f--) {
        set_bit(attacks, (8*target_rank + f));
        if (get_bit(blocked, (8*target_rank+f))) {
            break;
        }
    }
    for (f = target_file+1; f <= 7; f++) {
        set_bit(attacks, (8*target_rank + f));
        if (get_bit(blocked, (8*target_rank+f))) {
            break;
        }
    }
    return attacks;
}

// set occupancies

U64 set_occupancies(int index, int bits_in_mask, U64 attack_mask) {
    /* Every index can be signifed*/
    U64 occupancy = 0ULL;
    for (int i = 0; i < bits_in_mask; i++) {
        int square = first_bit(attack_mask);
        remove_bit(attack_mask, square);
        if (index & (1 << i)) {
            set_bit(occupancy, square);
        }
    }
    return occupancy;
}
int main() {

    initialize_leaper_attacks();
    // print_board(pawn_attacks[WHITE][h1]);
    // print_board(mask_rook_occupancies(a3));
    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            int square = 8*r+f;
            printf(" %d,", count_bits(mask_rook_occupancies(square)));
        }
        printf("\n");
    }
    return 0;
}