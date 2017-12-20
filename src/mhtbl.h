#ifndef __MHASHTBL_H_
#define __MHASHTBL_H_

#include "obvdb.h"

#define __MHASHTBL_ERR_SUCCESS_      0
#define __MHASHTBL_ERR_NO_ALLOCATE_  1
#define __MHASHTBL_ERR_THRD_JOIN_    2

#define __MHASHTBL_LOAD_FACTOR_      0.5
#define __MHASHTBL_BKT_DEFAULT_SZ_   16

typedef struct {
	uint32_t* _obvs;  /* indices of observations in this bucket */
	uint32_t*  _sig;  /* minhash signature shared by all observations in this bucket */
	uint32_t    _sz;  /* size of the array containing observation indices */
	uint32_t   _cnt;  /* number of observations in this bucket */
} _bkt;

typedef struct {
	uint32_t      _B;  /* number bands (OR) */
	uint32_t      _R;  /* number rows (AND) */
	obvdb*    _refdb;  /* observation database used for construction */
	uint32_t*    _Hs;  /* H_1, H_2, ..., H_(BxR); H_i : F -> perm(F) */
	_bkt**       _Ts;  /* minhash tables T_1, T_2, ..., T_B; each of size szs[i] */
	uint32_t    _Tsz;  /* size of each minhash table T_b in Ts */
} mhtbl;

/* Initializes the given minhash table. */
uint32_t mhtbl_init(mhtbl* mht, obvdb* db, uint32_t bands, uint32_t rows, uint32_t seed);

/* Frees all memory used by the given minhash table. */
void mhtbl_destroy(mhtbl* mht);

/** Queries the given minhash table with the observations in
 *  the given database. Identified pairs are then outputted to
 *  the specified file. */
uint32_t mhtbl_query(mhtbl* mht, obvdb* qdb, uint32_t thrdcnt, FILE* outfile, uint32_t excl_mode, uint32_t no_print, uint32_t verbose);

#endif
