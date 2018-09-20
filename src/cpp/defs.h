#ifndef DEFS_H
#define DEFS_H

#include "stdlib.h"
#include <unordered_map>
using namespace std;


// #define DEBUG

#ifndef DEBUG
#define ASSERT(n)
#else
#define ASSERT(n) \
if(!(n)) { \
printf("%s - Failed",#n); \
printf("On %s ",__DATE__); \
printf("At %s ",__TIME__); \
printf("In File %s ",__FILE__); \
printf("At Line %d\n",__LINE__); \
exit(1);}
#endif

typedef unsigned long long U64;
#define BRD_SQ_NUM 120

#define START_FEN  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define MAXGAMEMOVES 2048
#define MAXPOSITIONMOVES 256
#define MAXDEPTH 64

#define ISMATE 10000



/* GAME MOVE */

/*                         	                        
0000 0000 0000 0000 0000 0111 1111 -> From 0x7F
0000 0000 0000 0011 1111 1000 0000 -> To >> 7, 0x7F
0000 0000 0011 1100 0000 0000 0000 -> Captured >> 14, 0xF
0000 0000 0100 0000 0000 0000 0000 -> EP 0x40000
0000 0000 1000 0000 0000 0000 0000 -> Pawn Start 0x80000
0000 1111 0000 0000 0000 0000 0000 -> Promoted Piece >> 20, 0xF
0001 0000 0000 0000 0000 0000 0000 -> Castle 0x1000000
*/

#define FROMSQ(m) ((m) & 0x7F)
#define TOSQ(m) (((m)>>7) & 0x7F)
#define CAPTURED(m) (((m)>>14) & 0xF)
#define PROMOTED(m) (((m)>>20) & 0xF)

#define MFLAGEP 0x40000
#define MFLAGPS 0x80000
#define MFLAGCA 0x1000000

#define MFLAGCAP 0x7C000
#define MFLAGPROM 0xF00000

#define NOMOVE 0

extern U64 BlackPassedMask[64];
extern U64 WhitePassedMask[64];
extern U64 IsolatedMask[64];
extern U64 SetMask[64];
extern U64 ClearMask[64];
extern U64 FileBBMask[8];
extern U64 RankBBMask[8];

enum { EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK  };
enum { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NONE };
enum { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NONE };

// 1 0 0 1, means that white can cascal king side, not queen side, and black can castle king side not queen side
enum { WKCA = 1, WQCA = 2, BKCA = 4, BQCA = 8 };		// castling

enum { WHITE, BLACK, BOTH };
enum {
  A1 = 21, B1, C1, D1, E1, F1, G1, H1,
  A2 = 31, B2, C2, D2, E2, F2, G2, H2,
  A3 = 41, B3, C3, D3, E3, F3, G3, H3,
  A4 = 51, B4, C4, D4, E4, F4, G4, H4,
  A5 = 61, B5, C5, D5, E5, F5, G5, H5,
  A6 = 71, B6, C6, D6, E6, F6, G6, H6,
  A7 = 81, B7, C7, D7, E7, F7, G7, H7,
  A8 = 91, B8, C8, D8, E8, F8, G8, H8, NO_SQ, OFFBOARD
};

enum { FALSE, TRUE };

typedef struct {
	int move;
	int castlePerm;
	int enPas;
	int fiftyMove;
	U64 posKey;

} S_UNDO;	// used for history

typedef struct {
	int move;
	int score;
} S_MOVE;

typedef struct {
	S_MOVE moves[MAXPOSITIONMOVES];
	int count;
} S_MOVELIST;


typedef struct {
	U64 posKey;
	int move;
} S_PVENTRY;

typedef struct {
	S_PVENTRY *pTable;
	int numEntries;
} S_PVTABLE;

typedef struct {

	int starttime;
	int stoptime;
	int depth;
	int timeset;
	int movestogo;
	
	long nodes;
	
	int quit;
	int stopped;
	
	float fh;
	float fhf;

} S_SEARCHINFO;

typedef struct {
	U64 posKey;
	int move;
	int score;
	int depth;
	int flags;
} S_HASHENTRY;

struct S_HASHENTRY_V2 {
	int move;
	int score;
	int depth;
	int flags;

	S_HASHENTRY_V2():depth(0)
    {
    }
};

typedef struct {
	S_HASHENTRY *pTable;
	int numEntries;
	int newWrite;
	int overWrite;
	int hit;
	int cut;
} S_HASHTABLE;

typedef struct {

	int pieces[BRD_SQ_NUM];
	U64 pawns[3]; // index 0 for the white pawns, index 1 for the black pawns, index 2 for both

	int KingSq[2];

	int side;			// which side to move
	int enPas;			// en posson sqr
	int fiftyMove; 		// at fityMove, game is draw

	int ply;			// how many half moves has been made
	int hisPly;			// hitory move

	U64 posKey;

	int pceNum[13]; 	// number of pieces, each index represents one type of piece
	int bigPce[2];
	int majPce[2];
	int minPce[2];
	int material[2];

	int castlePerm; 	// integer for castle permissions
	S_UNDO history[MAXGAMEMOVES];

	// piece list
	// used to track where are each piece on the boards
	// so instead of looping each piece and find the move it can play
	// we just generate moves through this one and
	// and it will be much faster
	int pList[13][10];

	// adding two knights, pList[wN][0] = E1;
	// adding two knights, pList[wN][0] = D4;

	int searchHistory[13][BRD_SQ_NUM];
	int searchKillers[2][MAXDEPTH];

	S_HASHTABLE HashTable[1];
	int PvArray[MAXDEPTH];



} S_BOARD;

/* MACROS */
#define FR2SQ(f,r) ( (21 + (f) ) + ( (r) * 10 ) ) // GIVE the file and rank, return the sqr idx for the stuff
#define SQ64(i_out_of_120) Sq120ToSq64[i_out_of_120]	// return the idx of the sq64 board
#define SQ120(i_out_of_64) Sq64ToSq120[i_out_of_64]	// return the idx of the sq120 board
#define POP(b) PopBit(b)
#define CNT(b) CountBits(b)
#define CLRBIT(bb,sq) ((bb) &= ClearMask[(sq)])
#define SETBIT(bb,sq) ((bb) |= SetMask[(sq)])

#define IsBQ(p) (PieceBishopQueen[(p)])
#define IsRQ(p) (PieceRookQueen[(p)])
#define IsKn(p) (PieceKnight[(p)])
#define IsKi(p) (PieceKing[(p)])


extern int PieceBig[13];
extern int PieceMaj[13];
extern int PieceMin[13];
extern int PieceVal[13];
extern int PieceCol[13];
extern int PiecePawn[13];

extern int FilesBrd[BRD_SQ_NUM];
extern int RanksBrd[BRD_SQ_NUM];

/* GLOBALS */
extern int Sq120ToSq64[BRD_SQ_NUM];
extern int Sq64ToSq120[64];
extern U64 SetMask[64];
extern U64 ClearMask[64];
extern U64 PieceKeys[13][120];
extern U64 SideKey;
extern U64 CastleKeys[16];
extern char PceChar[];
extern char SideChar[];
extern char RankChar[];
extern char FileChar[];

extern int PieceKnight[13];
extern int PieceKing[13];
extern int PieceRookQueen[13];
extern int PieceBishopQueen[13];
extern int PieceSlides[13];

/* functions */
extern void AllInit(void);
extern void InitSq120To64(void);
extern void PrintBitBoard(U64 bb);
extern int PopBit(U64 *bb);
extern int CountBits(U64 b);
extern void InitBitMasks(void);
extern U64 GeneratePosKey(const S_BOARD *pos);
extern void ResetBoard(S_BOARD *pos);
extern int ParseFen(char *fen, S_BOARD *pos);
extern void PrintBoard(const S_BOARD *pos);
extern int CheckBoard(const S_BOARD *pos);
extern int SqAttacked(const int sq, const int side, const S_BOARD *pos);

// movegen.c
extern void GenerateAllMoves(S_BOARD *pos, S_MOVELIST *list);
extern void GenerateAllCaps(S_BOARD *pos, S_MOVELIST *list);
extern int MoveExists(S_BOARD *pos, const int move);
extern void InitMvvLva(void);
extern int MakeMove(S_BOARD *pos, int move);
extern void TakeMove(S_BOARD *pos);
extern int MoveIsIntoCheck(S_BOARD *pos, const int move);

// validate.c
extern int SqOnBoard(const int sq);
extern int SideValid(const int side);
extern int FileRankValid(const int fr);
extern int PieceValidEmpty(const int pce);
extern int PieceValid(const int pce);

// io.c
extern char *PrMove(const int move);
extern char *PrSq(const int sq);
extern void PrintMoveList(const S_MOVELIST *list);
extern int ParseMove(char *ptrChar, S_BOARD *pos);

// misc.c
extern int GetTimeMs(void);
extern void ReadInput(S_SEARCHINFO *info);

extern void PerftTest(int depth, S_BOARD *pos);

//evaluate.c
#define ENDGAME_MAT (1 * PieceVal[wR] + 2 * PieceVal[wN] + 2 * PieceVal[wP] + PieceVal[wK])
extern int evaluation(const S_BOARD* pos);

// main.c
#define WIN_SCORE 10000
#define LOSS_SCORE -10000
#define DRAW_SCORE 0

// PyObject* moveScoreList(PyObject *, PyObject *);

// pvtable.c
extern void InitHashTable(S_HASHTABLE *table, const int MB);
extern void StoreHashEntry(S_BOARD *pos, const int move, int score, const int flags, const int depth);
extern int ProbeHashEntry(S_BOARD *pos, int *move, int *score, int *alpha, int *beta, int depth);
extern int ProbePvMove(const S_BOARD *pos);
extern int GetPvLine(const int depth, S_BOARD *pos);
extern void ClearHashTable(S_HASHTABLE *table);
enum {  HFNONE, HFALPHA, HFBETA, HFEXACT};

// hashtable.c
extern unordered_map<U64, struct S_HASHENTRY_V2> InitHashTable();
extern int ProbeHashEntry_V2(S_BOARD *pos, int *move, int *score, int *alpha, int *beta, int depth, unordered_map<U64, struct S_HASHENTRY_V2> &m);
extern int ProbePvMove_V2(const S_BOARD *pos, unordered_map<U64, struct S_HASHENTRY_V2> &m);
extern void StoreHashEntry_V2(S_BOARD *pos, const int move, int score, const int flags, const int depth, unordered_map<U64, struct S_HASHENTRY_V2> &m);


extern int Mirror64[64];
#define MIRROR64(sq) (Mirror64[(sq)])

#endif
