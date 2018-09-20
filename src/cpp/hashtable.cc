#include "stdio.h"
#include "defs.h"

unordered_map<U64, struct S_HASHENTRY_V2> InitHashTable() {

	unordered_map<U64, struct S_HASHENTRY_V2> m;
	// int size = 0x100000 * 2000;
	// int entries = size / sizeof(S_HASHENTRY_V2);
	m.reserve(30000000);
	// for (auto it: m) {
	// 	// it.first = 0ULL;
	// 	// it.second.posKey = 0ULL;
 //    	it.second.move = NOMOVE;
 //    	it.second.depth = 0;
 //    	it.second.score = 0;
 //    	it.second.flags = 0;
	// }
	return m;
}

int ProbeHashEntry_V2(S_BOARD *pos, int *move, int *score, int *alpha, int *beta, int depth, unordered_map<U64, struct S_HASHENTRY_V2> &m) {
	U64 curPosKey = pos->posKey;
	unordered_map<U64, struct S_HASHENTRY_V2>::const_iterator it = m.find(curPosKey);
	if (it == m.end()) {
		return FALSE;
	} else {
		*move = it->second.move;
		if (it->second.depth >= depth) {
			pos->HashTable->hit++;
			*score = it->second.score;
			switch(it->second.flags) {
                case HFALPHA: if(*score<=*alpha) {
                    *score=*alpha;
                    return TRUE;
                    } else {
                    	if (*score < *beta)
                    		*beta = *score;
                    }
                    break;
                case HFBETA: if(*score>=*beta) {
                    *score=*beta;
                    return TRUE;
                    } else {
                    	if (*score >  *alpha)
                    		*alpha = *score;
                    }
                    break;
                case HFEXACT:
                    return TRUE;
                    break;
                default: ASSERT(FALSE); break;
			}
		}
	}
	return FALSE;
}

void StoreHashEntry_V2(S_BOARD *pos, const int move, int score, const int flags, const int depth, unordered_map<U64, struct S_HASHENTRY_V2> &m) {
	U64 curPosKey = pos->posKey;


	// unordered_map<U64,S_HASHENTRY_V2>::const_iterator it = m.find(curPosKey);

	// If the key does exist in the dictionary
	// if(it != m.end()){
		if (m[curPosKey].depth < depth) {
			m[curPosKey].flags = flags;
			m[curPosKey].score = score;
			m[curPosKey].depth = depth;
			m[curPosKey].move = move;
			// pos->HashTable->overWrite++;
		// }

	// }

	// If its a new key
	// else{
		// S_HASHENTRY_V2 s;
		// s.move = move;
		// s.flags = flags;
		// s.score = score;
		// s.depth = depth;
	    // m[curPosKey] = s;
		pos->HashTable->newWrite++;
	}
}

int ProbePvMove_V2(const S_BOARD *pos, unordered_map<U64, struct S_HASHENTRY_V2> &m) {

	U64 curPosKey = pos->posKey;

	ASSERT(index >= 0 && index <= pos->HashTable->numEntries - 1);
	return m[curPosKey].move;

	// return NOMOVE;
}


