# cppchess
New chess engine in C++.

This is my very first full-fledged chess engine in C++. As an intermediate level chess player, I wanted to try to build a chess engine from scratch. 

This chess engine uses bitboards, in which our board is stored as an unsigned 64-bit integer. 

I'd like to give credit to the Chess Programming Wiki in explaining all these new concepts such as attack maps, bitboards, and magics, as well as the YouTube Channel Chess Programming, whose Bitboard Chess Engine in C playlist was a monumental help for me in getting started and learning about bitboards.

MAGIC BITBOARDS:

"Magic bitboards" describe an algorithm for perfectly hashing different occupancy bitboards. By occupancy bitboards, we mean a mask of all positions that COULD affect a sliding piece's (rook/bishop) movement. (Actually, this is why we don't include the edge squares, they don't actually prevent us from moving anywhere!)

Consider the following occupancy mask of a bishop on d4:

0 0 0 0 0 0 0 0

0 0 0 0 0 0 1 0

0 1 0 0 0 1 0 0

0 0 1 0 1 0 0 0

0 0 0 0 0 0 0 0

0 0 1 0 1 0 0 0

0 1 0 0 0 1 0 0

0 0 0 0 0 0 0 0

Again, the 1s here do NOT signify the bishop's attacks, but rather the squares any OTHER piece could be on that could impede the bishop's movements. Now, each of the bit positions corresponding to 1s could be 1 or 0, corresponding to whether a piece is indeed on that square or not. For example, in a real game, the actual occupancies might look like this:

0 0 0 0 0 0 0 0

0 0 0 0 0 0 1 0

0 1 0 0 0 0 0 0

0 0 0 0 0 0 0 0

0 0 0 0 0 0 0 0

0 0 1 0 1 0 0 0

0 1 0 0 0 0 0 0

0 0 0 0 0 0 0 0

Now, manually checking the diagonals every single time we want to make a bishop move can get ridiculously expensive, especially if we want to be searching potentially millions of positions in our engine. 

This is where MAGIC BITBOARDS come in. The key idea of magic bitboards is that there are $2^9=512$ different occupancies for this one square, since each of the 9 '1' positions in the full occupancy mask could either be zero or $1$. Then we find a magic number $M$ such that multiplying that bitboard by $M$ results in a board in which all the bits corresponding to the occupancy squares lie on the bottom row, after which we can right shift easily. For example,


0 0 0 0 0 0 0 0 

0 0 0 0 0 0 A 0

0 B 0 0 0 C 0 0

0 0 D 0 E 0 0 0  * $M$  ->    

0 0 0 0 0 0 0 0

0 0 F 0 G 0 0 0

0 H 0 0 0 I 0 0

0 0 0 0 0 0 0 0

becomes

0 0 0 0 0 0 0 0

0 0 0 0 0 0 0 0

0 0 0 0 0 0 0 0

0 0 garbage 0 0

0 0 0 0 0 0 0 0

0 0 0 0 0 0 0 0

0 0 0 0 0 0 0 A

B C D E F G H I


Thus, we precalculate the moves corresponding to any possible occupancy and any possible square (leading to $64\cdot 512=32768$ different bitboards, a good enough space tradeoff for extremely fast move generation), and we can use this magic hashing algorithm to quickly turn this occupancy mask into an index which we can look up in a precalculated table. 

