#include <stdio.h>
#include <stdlib.h>     /* qsort */
#include "defs.h"

// using namespace std;

int BRANCH_REDUCE_FACTOR = 3;
int REDUCE_DEPTH = 4;
int SEARCH_DEPTH = 10;
U64 rootPoskey;

#define UPPER_BOUND_FLAG 0
#define LOWER_BOUND_FLAG 1
#define EXACT_FLAG 2
#define NOTFOUND -1000000000
#define MAXDEPTH 64

#define HASH_PCE(pce,sq) (pos->posKey ^= (PieceKeys[(pce)][(sq)]))
#define HASH_CA (pos->posKey ^= (CastleKeys[(pos->castlePerm)]))
#define HASH_SIDE (pos->posKey ^= (SideKey))
#define HASH_EP (pos->posKey ^= (PieceKeys[EMPTY][(pos->enPas)]))


struct INFO
{
	int node_count;
	int stored;
};

static void ClearForSearch(S_BOARD *pos) {

	int index = 0;
	int index2 = 0;

	for(index = 0; index < 13; ++index) {
		for(index2 = 0; index2 < BRD_SQ_NUM; ++index2) {
			pos->searchHistory[index][index2] = 0;
		}
	}

	for(index = 0; index < 2; ++index) {
		for(index2 = 0; index2 < MAXDEPTH; ++index2) {
			pos->searchKillers[index][index2] = 0;
		}
	}
}

static int IsRepetition(const S_BOARD *pos) {
	int index = 0;
	for(index = pos->hisPly - pos->fiftyMove; index < pos->hisPly-1; ++index) {
		ASSERT(index >= 0 && index < MAXGAMEMOVES);
		if(pos->posKey == pos->history[index].posKey) {
			return TRUE;
		}
	}
	return FALSE;
}

static void PickNextMove(int moveNum, S_MOVELIST *list) {
	S_MOVE temp;
	int index = 0;
	int bestScore = 0;
	int bestNum = moveNum;

	for (index = moveNum; index < list->count; ++index) {
		if (list->moves[index].score > bestScore) {
			bestScore = list->moves[index].score;
			bestNum = index;
		}
	}

	ASSERT(moveNum>=0 && moveNum<list->count);
	ASSERT(bestNum>=0 && bestNum<list->count);
	ASSERT(bestNum>=moveNum);

	temp = list->moves[moveNum];
	list->moves[moveNum] = list->moves[bestNum];
	list->moves[bestNum] = temp;
}

int min(int a, int b) {
	return a <= b ? a : b;
}

int max(int a, int b) {
	return a >= b ? a : b;
}

void MakeNullMove(S_BOARD *pos) {

    ASSERT(CheckBoard(pos));
    ASSERT(!SqAttacked(pos->KingSq[pos->side],pos->side^1,pos));

    pos->ply++;
    pos->history[pos->hisPly].posKey = pos->posKey;

    if(pos->enPas != NO_SQ) HASH_EP;

    pos->history[pos->hisPly].move = NOMOVE;
    pos->history[pos->hisPly].fiftyMove = pos->fiftyMove;
    pos->history[pos->hisPly].enPas = pos->enPas;
    pos->history[pos->hisPly].castlePerm = pos->castlePerm;
    pos->enPas = NO_SQ;

    pos->side ^= 1;
    pos->hisPly++;
    HASH_SIDE;
   
    ASSERT(CheckBoard(pos));
	ASSERT(pos->hisPly >= 0 && pos->hisPly < MAXGAMEMOVES);
	ASSERT(pos->ply >= 0 && pos->ply < MAXDEPTH);

    return;
} // MakeNullMove

void TakeNullMove(S_BOARD *pos) {
    ASSERT(CheckBoard(pos));

    pos->hisPly--;
    pos->ply--;

    if(pos->enPas != NO_SQ) HASH_EP;

    pos->castlePerm = pos->history[pos->hisPly].castlePerm;
    pos->fiftyMove = pos->history[pos->hisPly].fiftyMove;
    pos->enPas = pos->history[pos->hisPly].enPas;

    if(pos->enPas != NO_SQ) HASH_EP;
    pos->side ^= 1;
    HASH_SIDE;
  
    ASSERT(CheckBoard(pos));
	ASSERT(pos->hisPly >= 0 && pos->hisPly < MAXGAMEMOVES);
	ASSERT(pos->ply >= 0 && pos->ply < MAXDEPTH);
}

int Quiescence(S_BOARD *pos, int alpha, int beta, int colour, struct INFO* info) {
	int MoveNum = 0;
	int Legal = 0;
	int Score;
	S_MOVELIST list[1];

	ASSERT(CheckBoard(pos));
	ASSERT(beta > alpha);

	info->node_count++;

	if(IsRepetition(pos) || pos->fiftyMove >= 100) {
		return 0;
	}

	if(pos->ply > MAXDEPTH - 1)
		return evaluation(pos) * colour;

	Score = evaluation(pos) * colour;

	if(Score >= beta)
		return beta;

	if(Score > alpha)
		alpha = Score;

	GenerateAllCaps(pos, list);

	Score = LOSS_SCORE - 1;

	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {
		PickNextMove(MoveNum, list);

		if (!MakeMove(pos, list->moves[MoveNum].move))
			continue;

		Legal++;
		Score = -Quiescence(pos, -beta, -alpha, -colour, info);
		TakeMove(pos);

		if(Score > alpha) {
			if(Score >= beta) {
				return beta;
			}
			alpha = Score;
		}
	}

	return alpha;
}


int AlphaBeta(S_BOARD *pos, int alpha, int beta, int depth, int colour, struct INFO* info, int DoNull, unordered_map<U64, struct S_HASHENTRY_V2> &m) {

	if((IsRepetition(pos) || pos->fiftyMove >= 100) && pos->ply) {
		return 0;
	}

	// S_PVENTRY* entry = ProbePvTable(pos);
	// info->node_count ++;
	// if (entry != NULL && entry->depth >= depth) {
	// 	info->stored += 1;
	// 	return entry->score;
	// }

	// base case
	if (depth <= 0) {
		int score = Quiescence(pos, alpha, beta, colour, info);;
		return score;
	}

	int InCheck = SqAttacked(pos->KingSq[pos->side], pos->side ^ 1, pos);

	if (InCheck == TRUE) {
		depth ++;
	}

	if( DoNull && !InCheck && pos->ply && (pos->bigPce[pos->side] > 0) && depth >= 4) {
		MakeNullMove(pos);
		int Score = -AlphaBeta(pos, -beta, -beta + 1, depth-4, -colour, info, FALSE, m);
		TakeNullMove(pos);

		if (Score >= beta) {
			return beta;
		}
	}

	S_MOVELIST moves[1];
	int legalMovesCount = 0;
	int OldAlpha = alpha;
	int move;
	int bestScore = LOSS_SCORE - 1;		// make sure that this will be updated
	int bestMove = 0;
	int curScore;
	GenerateAllMoves(pos, moves);

	// use pv move to help
	// int Pvmove = ProbePvTable(pos);
	int Score = -WIN_SCORE;
	int Pvmove = NOMOVE;

	if( ProbeHashEntry_V2(pos, &Pvmove, &Score, &alpha, &beta, depth, m) == TRUE ) {
		pos->HashTable->cut++;
		return Score;
	}
	if(Pvmove != NOMOVE) {
		for(int MoveNum = 0; MoveNum < moves->count; ++MoveNum) {
			if( moves->moves[MoveNum].move == Pvmove) {
				moves->moves[MoveNum].score = 5000000;
				// printf("Pv move found \n");		
				break;
			}
		}
	}

	for (int i = 0; i < moves->count; ++i) {
		PickNextMove(i, moves);
		move = moves->moves[i].move;
		// if (pos->posKey == rootPoskey && i==0 && depth == 2) {
		// 	// printf("%s\n", PrMove(move));
		// 	printf("%s\n", "start");

		// 	for (int i=0; i < moves->count; i++) {
		// 		printf("%s\n", PrMove(moves->moves[i].move));
		// 	}
		// }

		if (!MakeMove(pos, move)) {
			continue;
		}

		legalMovesCount += 1;


		
		if (legalMovesCount >= 4 && depth > 2 && InCheck == FALSE && CAPTURED(move) == EMPTY && PROMOTED(move) == EMPTY) { // reduction
			curScore = -AlphaBeta(pos, -alpha-1, -alpha, depth - REDUCE_DEPTH, -colour, info, TRUE, m);
			// if (curScore > alpha) {
			// 	curScore = -AlphaBeta(pos, -beta, -alpha, depth-1, -colour, info, TRUE);
			// }
		} else  {
			curScore = alpha + 1;
		}

		if (curScore > alpha) {
			if (legalMovesCount == 1 || InCheck == TRUE || CAPTURED(move) != EMPTY || PROMOTED(move) != EMPTY){
				curScore = -AlphaBeta(pos, -beta, -alpha, depth-1, -colour, info, TRUE, m);
			} else {
				curScore = -AlphaBeta(pos, -alpha-1, -alpha, depth-1, -colour, info, TRUE, m);
				if (curScore > alpha && curScore < beta) {
					curScore = -AlphaBeta(pos, -beta, -alpha, depth-1, -colour, info, TRUE, m);
				}
			}
		}

		TakeMove(pos);
		if (curScore > bestScore) {
			bestScore = curScore;
			bestMove = move;
		}
		if(bestScore > alpha) {
			alpha = bestScore;
			if (alpha >= beta) {
				// cutoff
				if(!(move & MFLAGCAP)) {
					pos->searchKillers[1][pos->ply] = pos->searchKillers[0][pos->ply];
					pos->searchKillers[0][pos->ply] = move;
				}
				StoreHashEntry_V2(pos, bestMove, beta, HFBETA, depth, m);
				return beta;
			}
			if(!(move & MFLAGCAP)) {
				pos->searchHistory[pos->pieces[FROMSQ(bestMove)]][TOSQ(bestMove)] += depth;
			}
		}

	}


	if (legalMovesCount == 0) {
		if (InCheck == TRUE) {						// checkmate
			bestScore = LOSS_SCORE;					// LOSS THE GAME
		} else {									// stalemate, draw
			bestScore = DRAW_SCORE;
		}
	}

	// b.score = bestScore;
	// if (bestScore <= OldAlpha) {
 //        b.flag = UPPER_BOUND_FLAG;
	// } else if (bestScore >= beta) {
 //        b.flag = LOWER_BOUND_FLAG;
	// } else {
 //        b.flag = EXACT_FLAG;
	// }
	// b.depth = depth;
	// result_dict[pos->posKey] = b;
	// StoreSearchResult(pos, depth, bestScore);
	if(alpha != OldAlpha) {
		StoreHashEntry_V2(pos, bestMove, bestScore, HFEXACT, depth, m);
	} else {
		// StoreHashEntry(pos, bestMove, alpha, HFALPHA, depth);
		StoreHashEntry_V2(pos, bestMove, alpha, HFALPHA, depth, m);
	}

	return bestScore;
}


// #define python_wrapper

#ifdef python_wrapper
	#include <Python.h>

	PyObject * next_move(PyObject *self, PyObject *args) {
		char *game_fen, *next_move;
		int depth;

		if(!PyArg_ParseTuple(args, "si", &game_fen, &depth))
			return NULL;

		AllInit();
		S_BOARD board[1];
		InitPvTable(board->PvTable);
		ParseFen(game_fen, board);
		for (int i=1; i <= depth; i++) {
			printf("%d\n", AlphaBeta(board, LOSS_SCORE-1, WIN_SCORE+1, i, -1));
		}
		next_move = PrMove(ProbePvTable(board));
		return Py_BuildValue("s", next_move);

	}

	//Method definition object for this extension, these argumens mean:
	//ml_name: The name of the method
	//ml_meth: Function pointer to the method implementation
	//ml_flags: Flags indicating special features of this method, such as
	//          accepting arguments, accepting keyword arguments, being a
	//          class method, or being a static method of a class.
	//ml_doc:  Contents of this method's docstring
	static PyMethodDef chess_methods[] = {
	    {
	        "next_move",
	        next_move,
	        METH_VARARGS,
	        "evaluate a chess board position from fen string"
	    },
	    {NULL, NULL, 0, NULL}
	};


	//Module definition
	//The arguments of this structure tell Python what to call your extension,
	//what it's methods are and where to look for it's method definitions
	static struct PyModuleDef chess_definition = {
	    PyModuleDef_HEAD_INIT,
	    "evaluation",
	    "A Python module that provides next move string given a fen",
	    -1,
	    chess_methods
	};

	//Module initialization
	//Python calls this function when importing your extension. It is important
	//that this function is named PyInit_[[your_module_name]] exactly, and matches
	//the name keyword argument in setup.py's setup() call.
	PyMODINIT_FUNC PyInit_chess_in_c(void)
	{
	    Py_Initialize();
	    return PyModule_Create(&chess_definition);
	}

#else
	int main(int argc, char const *argv[])
	{
		/* code */
		AllInit();

		// unordered_map<U64, struct board_result> result_dict;
		// result_dict.reserve(130000);
		S_BOARD board[1];
		char* fen = (char*)argv[1];
		const int side = atoi(argv[2]); // white == 0, black == 1
		ParseFen(fen, board);

		// create some hashing tables
		InitHashTable(board->HashTable, 1);
		unordered_map<U64, struct S_HASHENTRY_V2> m = InitHashTable();
		// InitPvTable(board->PvTable);
		struct INFO info;
		info.node_count = 0;
		info.stored = 0;

		// set some search parameters
		if (board->material[board->side] <= ENDGAME_MAT) {	// END GAME
			printf("End Game\n");
			BRANCH_REDUCE_FACTOR = 1;
			REDUCE_DEPTH = 2;
			SEARCH_DEPTH = 12;
		} else {
			printf("NOT End Game\n");			// NOT ENDING
			BRANCH_REDUCE_FACTOR = 1;
			REDUCE_DEPTH = 2;
			SEARCH_DEPTH = 10;
		}

		rootPoskey = board->posKey;
		ClearForSearch(board);
		int alpha = LOSS_SCORE-1;
		int beta = WIN_SCORE+1;
		int window = 10;
		int colour = side == WHITE ? 1 : -1;
		int lastScore = 0;
		// check the game status to determine what parameter we should set
		for (int i=1; i <=SEARCH_DEPTH; i++) {
			if (lastScore == WIN_SCORE || lastScore == LOSS_SCORE){
				break;
			}
			if (lastScore <= alpha || lastScore >= beta) {
				alpha = LOSS_SCORE-1;
				beta = WIN_SCORE+1;
				info.node_count = 0;
				info.stored = 0;
				board->HashTable->hit = 0;
				i -= 1;
			} else {
				alpha = lastScore - window;
				beta = lastScore + window;
			}
			lastScore = AlphaBeta(board, alpha, beta, i, colour, &info, TRUE, m);
			printf("depth: %d, score: %d, node_count: %d, alpha: %d, beta: %d, hit: %d, new: %d, overwrite: %d\n", i, lastScore, info.node_count, alpha, beta, board->HashTable->hit, board->HashTable->newWrite, board->HashTable->overWrite);
			info.node_count = 0;
			info.stored = 0;
			board->HashTable->hit = 0;
			// board->HashTable->overWrite = 0;
			// board->HashTable->newWrite = 0;

		}
		printf("%s\n", PrMove(ProbePvMove_V2(board, m)));
		// ASSERT(CheckBoard(board));
		return 0;
	}
#endif
