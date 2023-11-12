#include <stdio.h>
#include <string.h>
#include "magics.h"
typedef unsigned long long U64;
#define get_bit(bb, square) ((bb) & (1ULL << square))
#define set_bit(bb, square) ((bb) |= (1ULL << square))
#define remove_bit(bb, square) ((bb) &= ~(1ULL << square))
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
    a1, b1, c1, d1, e1, f1, g1, h1, no_sq
};
// Colors and Piecs
enum {WHITE, BLACK, BOTH};
enum {ROOK, BISHOP};

U64 piece_bitboards[12]; // One bitboard for each piece on white and black (12 total)
U64 occupancy_bitboards[3]; // Bitboards describing occupancies (positions of pieces of each color, and of both colors, total 3)
int side = -1;
int enpassant = no_sq;
int castle;
// Castling rights (Represented by four bits, each bit represents color/queen+kingside castling)
enum {WK = 1, WQ = 2, BK = 4, BQ = 8};
// Encode pieces
enum {P, N, B, R, Q, K, p, n, b, r, q, k};
char asciipieces[13] = "PNBRQKpnbrqk";
const char *piece_symbols[12] = {"♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚"};
int char_to_number[] = {
    ['P'] = P,
    ['N'] = N,
    ['B'] = B,
    ['R'] = R,
    ['Q'] = Q,
    ['K'] = K,
    ['p'] = p,
    ['n'] = n,
    ['b'] = b,
    ['r'] = r,
    ['q'] = q,
    ['k'] = k

};
// Masks of all squares not on the a-file or h-file, not a or b-file, and not g or h-files. Latter for knights.
const U64 not_A_file = 18374403900871474942;
const U64 not_H_file = 9187201950435737471;
const U64 not_GH_file = 4557430888798830399;
const U64 not_AB_file = 18229723555195321596;

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

// Precalculated Magic Numbers
const U64 rook_magics[64] = {
    0x8a80104000800020ULL,
    0x140002000100040ULL,
    0x2801880a0017001ULL,
    0x100081001000420ULL,
    0x200020010080420ULL,
    0x3001c0002010008ULL,
    0x8480008002000100ULL,
    0x2080088004402900ULL,
    0x800098204000ULL,
    0x2024401000200040ULL,
    0x100802000801000ULL,
    0x120800800801000ULL,
    0x208808088000400ULL,
    0x2802200800400ULL,
    0x2200800100020080ULL,
    0x801000060821100ULL,
    0x80044006422000ULL,
    0x100808020004000ULL,
    0x12108a0010204200ULL,
    0x140848010000802ULL,
    0x481828014002800ULL,
    0x8094004002004100ULL,
    0x4010040010010802ULL,
    0x20008806104ULL,
    0x100400080208000ULL,
    0x2040002120081000ULL,
    0x21200680100081ULL,
    0x20100080080080ULL,
    0x2000a00200410ULL,
    0x20080800400ULL,
    0x80088400100102ULL,
    0x80004600042881ULL,
    0x4040008040800020ULL,
    0x440003000200801ULL,
    0x4200011004500ULL,
    0x188020010100100ULL,
    0x14800401802800ULL,
    0x2080040080800200ULL,
    0x124080204001001ULL,
    0x200046502000484ULL,
    0x480400080088020ULL,
    0x1000422010034000ULL,
    0x30200100110040ULL,
    0x100021010009ULL,
    0x2002080100110004ULL,
    0x202008004008002ULL,
    0x20020004010100ULL,
    0x2048440040820001ULL,
    0x101002200408200ULL,
    0x40802000401080ULL,
    0x4008142004410100ULL,
    0x2060820c0120200ULL,
    0x1001004080100ULL,
    0x20c020080040080ULL,
    0x2935610830022400ULL,
    0x44440041009200ULL,
    0x280001040802101ULL,
    0x2100190040002085ULL,
    0x80c0084100102001ULL,
    0x4024081001000421ULL,
    0x20030a0244872ULL,
    0x12001008414402ULL,
    0x2006104900a0804ULL,
    0x1004081002402ULL
};
const U64 bishop_magics[64] = {
    0x40040844404084ULL,
    0x2004208a004208ULL,
    0x10190041080202ULL,
    0x108060845042010ULL,
    0x581104180800210ULL,
    0x2112080446200010ULL,
    0x1080820820060210ULL,
    0x3c0808410220200ULL,
    0x4050404440404ULL,
    0x21001420088ULL,
    0x24d0080801082102ULL,
    0x1020a0a020400ULL,
    0x40308200402ULL,
    0x4011002100800ULL,
    0x401484104104005ULL,
    0x801010402020200ULL,
    0x400210c3880100ULL,
    0x404022024108200ULL,
    0x810018200204102ULL,
    0x4002801a02003ULL,
    0x85040820080400ULL,
    0x810102c808880400ULL,
    0xe900410884800ULL,
    0x8002020480840102ULL,
    0x220200865090201ULL,
    0x2010100a02021202ULL,
    0x152048408022401ULL,
    0x20080002081110ULL,
    0x4001001021004000ULL,
    0x800040400a011002ULL,
    0xe4004081011002ULL,
    0x1c004001012080ULL,
    0x8004200962a00220ULL,
    0x8422100208500202ULL,
    0x2000402200300c08ULL,
    0x8646020080080080ULL,
    0x80020a0200100808ULL,
    0x2010004880111000ULL,
    0x623000a080011400ULL,
    0x42008c0340209202ULL,
    0x209188240001000ULL,
    0x400408a884001800ULL,
    0x110400a6080400ULL,
    0x1840060a44020800ULL,
    0x90080104000041ULL,
    0x201011000808101ULL,
    0x1a2208080504f080ULL,
    0x8012020600211212ULL,
    0x500861011240000ULL,
    0x180806108200800ULL,
    0x4000020e01040044ULL,
    0x300000261044000aULL,
    0x802241102020002ULL,
    0x20906061210001ULL,
    0x5a84841004010310ULL,
    0x4010801011c04ULL,
    0xa010109502200ULL,
    0x4a02012000ULL,
    0x500201010098b028ULL,
    0x8040002811040900ULL,
    0x28000010020204ULL,
    0x6000020202d0240ULL,
    0x8918844842082200ULL,
    0x4010011029020020ULL
};
// Attack Maps

U64 pawn_attacks[2][64];
U64 knight_attacks[64];
U64 king_attacks[64];

U64 bishop_occupancy_masks[64];
U64 rook_occupancy_masks[64];
U64 bishop_attacks[64][512];
U64 rook_attacks[64][4096];

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

void print_game_state() {
    printf("\n");
    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            int sq = 8*r+f;
            if (f == 0) {
                printf(" %d  ", 8-r); // print ranks
            }
            int piece = -1;
            for (int p = 0; p < 11; p++) {
                if (get_bit(piece_bitboards[p], sq)) {
                    piece = p;
                }
            }
            printf(" %c", (piece == -1) ? '.' : asciipieces[piece]);
        }
        printf("\n");
    }
    printf("     a b c d e f g h \n");
    printf("\n Side:    %s", !side ? "White" : "Black");
    printf("\n Castling rights:   %c%c%c%c", (castle & WK) ? 'K' : '-', 
                                            (castle & WQ) ? 'Q' : '-',
                                            (castle & BK) ? 'k' : '-',
                                            (castle & BQ) ? 'q' : '-');
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

U64 set_occupancies(int index, int bits_in_mask, U64 occupancy_mask) {
    /* Every index can be signifed*/
    U64 occupancy = 0ULL;
    for (int i = 0; i < bits_in_mask; i++) {
        int square = first_bit(occupancy_mask);
        remove_bit(occupancy_mask, square);
        if (index & (1 << i)) {
            set_bit(occupancy, square);
        }
    }
    return occupancy;
}

U64 find_magic_number(int square, int num_occupancy_squares, int bishop) {
    // All possible occupancy masks
    U64 possible_occupancies[4096];
    std::mt19937 mt(time(NULL));
    // All attack masks for any occupancy
    U64 attacks[4096];
    
    U64 used_attacks[4096];

    U64 occupancy_mask = bishop ? mask_bishop_occupancies(square) : mask_rook_occupancies(square);
    int num_occupancy_indices = 1 << num_occupancy_squares;
    for (int i = 0; i < num_occupancy_indices; i++) {
        possible_occupancies[i] = set_occupancies(i, num_occupancy_squares, occupancy_mask);
        attacks[i] = bishop ? bishop_attacks_imm(square, possible_occupancies[i]) : rook_attacks_imm(square, possible_occupancies[i]);
    }

    for (int j = 0; j < 1000000000; j++) {
        U64 magic = magic_candidate();
        if (count_bits((magic * occupancy_mask) & 0xFF00000000000000) < 6) {
            continue;
        }
        // Initialize used_attacks with all 0s.
        memset(used_attacks, 0ULL, sizeof(used_attacks));
        int fail;
        for (int i = 0, fail = 0; !fail && i < num_occupancy_indices; i++) {
            int magic_index = (int)((possible_occupancies[i] * magic) >> (64 - num_occupancy_squares));
            if (used_attacks[magic_index] == 0ULL) {
                used_attacks[magic_index] = attacks[i];
            } else if (used_attacks[magic_index] != attacks[i]) {
                fail = 1;
            }
        }
        if (!fail) {
            return magic;
        }
    }
    printf("No magic number found.");
    return 0;
}

void init_magics() {
    for (int i = 0; i < 64; i++) {
        U64 magic = find_magic_number(i, bishop_occupancy_bit_count[i], BISHOP);
        printf("0x%llX,\n", magic);
    }
}

void initialize_slider_attacks(int bishop) {
    for (int i = 0; i < 64; i++) {
        bishop_occupancy_masks[i] = mask_bishop_occupancies(i);
        rook_occupancy_masks[i] = mask_rook_occupancies(i);

        U64 occupancy_mask = bishop ? bishop_occupancy_masks[i] : rook_occupancy_masks[i];
        int occupancy_mask_size = count_bits(occupancy_mask);
        int num_occupancy_indices = 1 << occupancy_mask_size;
        for (int j = 0; j < num_occupancy_indices; j++) {
            if (bishop) {
                U64 occupancy = set_occupancies(j, occupancy_mask_size, occupancy_mask);
                int magic_index = (occupancy * bishop_magics[i]) >> (64 - bishop_occupancy_bit_count[i]);
                bishop_attacks[i][magic_index] = bishop_attacks_imm(i, occupancy);
            } else {
                U64 occupancy = set_occupancies(j, occupancy_mask_size, occupancy_mask);
                int magic_index = (occupancy * rook_magics[i]) >> (64 - rook_occupancy_bit_count[i]);
                rook_attacks[i][magic_index] = rook_attacks_imm(i, occupancy);
            }
        }
    }
}

U64 get_bishop_attacks(int square, U64 occupancy) {
    occupancy &= bishop_occupancy_masks[square];
    occupancy *= bishop_magics[square];
    occupancy >>= (64 - bishop_occupancy_bit_count[square]);
    return bishop_attacks[square][occupancy];
}
U64 get_rook_attacks(int square, U64 occupancy) {
    occupancy &= rook_occupancy_masks[square];
    occupancy *= rook_magics[square];
    occupancy >>= (64 - rook_occupancy_bit_count[square]);
    return rook_attacks[square][occupancy];
}



/* ********** INIT ALL ********** */

void init_all() {
    initialize_leaper_attacks();
    initialize_slider_attacks(ROOK);
    initialize_slider_attacks(BISHOP);
}



/* ********** MAIN DRIVER METHOD ********** */
int main() {

    init_all();
    set_bit(piece_bitboards[P], e4);
    side = 0;
    castle = 15;
    print_board(piece_bitboards[P]);
    print_game_state();
    return 0;
}