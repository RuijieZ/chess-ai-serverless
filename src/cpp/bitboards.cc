#include "stdio.h"
#include "defs.h"

const int BitTable[64] = {
  63, 30, 3, 32, 25, 41, 22, 33, 15, 50, 42, 13, 11, 53, 19, 34, 61, 29, 2,
  51, 21, 43, 45, 10, 18, 47, 1, 54, 9, 57, 0, 35, 62, 31, 40, 4, 49, 5, 52,
  26, 60, 6, 23, 44, 46, 27, 56, 16, 7, 39, 48, 24, 59, 14, 12, 55, 38, 28,
  58, 20, 37, 17, 36, 8
};

U64 SetMask[64];
U64 ClearMask[64];

void InitBitMasks() {
	int index =0;
	for(index =0; index < 64; index++) {
		SetMask[index] = 0ULL;
		ClearMask[index] = 0ULL;
	}

	for(index=0; index < 64; index++) {
		SetMask[index] |= (1ULL << index);
		ClearMask[index] = ~SetMask[index];
	}
}

// pop the least significant bit which is 1 and and set that bit to zero
// return the idx of that bit
int PopBit(U64 *bb) {
  U64 b = *bb ^ (*bb - 1);
  unsigned int fold = (unsigned) ((b & 0xffffffff) ^ (b >> 32));
  *bb &= (*bb - 1);
  return BitTable[(fold * 0x783a9b23) >> 26];
}

// return the number of 1s inside of the bitboard
int CountBits(U64 b) {
  int r;
  for(r = 0; b; r++, b &= b - 1);
  return r;
}



void PrintBitBoard(U64 bb) {
	U64 shiftMe = 1ULL;		// 1 represented in 64 bits
	int sq64;
	int sq120;
	for (int r = RANK_8; r >= RANK_1; --r) {
		printf("\n");
		for (int f = FILE_A; f <= FILE_H; ++f) {
			sq120 = FR2SQ(f, r);		// idx based on the 120 array
			sq64 = SQ64(sq120);			// idx based on the 64 array
			if ((shiftMe << sq64) & bb)
				printf("X");
			else
				printf("-");
		}
	}
}