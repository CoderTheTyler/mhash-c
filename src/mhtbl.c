#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include "obvdb.h"
#include "mhtbl.h"
#include "utils.h"

int compare(const void* a, const void* b) {
	return ((*((int*)a) > *((int*)b)) - (*((int*)a) < *((int*)b)));
}

uint32_t mhtbl_compare(uint32_t rows, uint32_t* mhs1, uint32_t* mhs2);
uint32_t mhtbl_hash(uint32_t* mhsig, uint32_t rows);
_bkt* mhtbl_getbucket(uint32_t rows, uint32_t F, _bkt* T, uint32_t Tsz, uint32_t* sigvtbl, uint32_t sigv, uint32_t* mhsig, uint32_t* H);

/* Initializes the given minhash table. */
uint32_t mhtbl_init(mhtbl* mht, obvdb* db, uint32_t bands, uint32_t rows, uint32_t seed) {
	
	uint32_t i, j, k, r, b, f, temp;
	
	uint32_t    F;
	uint32_t*  Hs;
	_bkt**     Ts;
	uint32_t  Tsz;
	
	uint32_t        N;  /* number of observations in database */
	uint32_t     sigv;  /* version of minhash signature */
	uint32_t* sigvtbl;  /* version table used for amortized O(f) feature membership test */
	uint32_t*   mhsig;  /* minhash signature */
	uint32_t*     obv;  /* current observation */
	uint32_t      osz;  /* size of current observation */
	_bkt*         bkt;  /* bucket in which to insert current observation */
	uint32_t* bktobvs;  /* bucket observation index array replacement */
	
	/* Get some info from database */
	F = obvdb_dimensionality(db);
	N = obvdb_cnt(db);
	
	
	/******************************
	 * Generate minhash functions *
	 ******************************/
	
	/* Allocate minhash function array */
	Hs = (uint32_t*) malloc((bands * rows * F) * sizeof(uint32_t));
	if(Hs == NULL) {
		printf("[ERROR]   Failed to allocate minhash function array!\n");
		return -400;
	}
	
	/* Initialize minhash functions */
	for(i = 0; i < bands * rows * F; i++)
		Hs[i] = i % F;
	rand_seed(seed);  /* use seed for reproducibility */
	for(i = 0; i < bands * rows; i++) {
		k = i * F;
		/* Fisher-Yates shuffle to generate permutations in O(F) time */
		for(f = F-1; f >= 1; f--) {
			j = rand_range(f) + k;
			temp = Hs[k+f];
			Hs[k+f] = Hs[j];
			Hs[j] = temp;
		}
	}
	
	
	/*************************
	 * Create minhash tables *
	 *************************/
	
	/* Compute static size of minhash tables */
	Tsz = 2;
	while(Tsz <= N / __MHASHTBL_LOAD_FACTOR_)
		Tsz *= 2;
	
	/* Allocate arrays and tables */
	Ts = (_bkt**) malloc(bands * sizeof(_bkt*));
	if(Ts == NULL)
		return __MHASHTBL_ERR_NO_ALLOCATE_;
	for(i = 0; i < bands; i++) {
		Ts[i] = (_bkt*) calloc(Tsz, sizeof(_bkt));
		if(Ts[i] == NULL)
			return __MHASHTBL_ERR_NO_ALLOCATE_;
	}
	
	
	/****************************
	 * Hash target observations *
	 ****************************/
	
	/* Allocate temporary minhash signature container. */
	mhsig = (uint32_t*) calloc(rows, sizeof(uint32_t));
	if(mhsig == NULL)
		return __MHASHTBL_ERR_NO_ALLOCATE_;
	
	/* Allocate signature version table. */
	sigvtbl = (uint32_t*) calloc(F, sizeof(uint32_t));
	if(sigvtbl == NULL)
		return __MHASHTBL_ERR_NO_ALLOCATE_;
	sigv = 1;
	
	/* Insert hashed observations into each table. */
	for(i = 0; i < N; i++) {
		/* Build lookup table to perform fast feature membership test. */
		for(j = 0, obv = obvdb_getobv(db, i), osz = obvdb_obvsz(db, i); j < osz; j++)
			sigvtbl[obv[j]] = sigv;
		
		/* Hash and insert observation i to all B minhash tables. */
		for(b = 0; b < bands; b++) {
			/* Get bucket for this observation in minhash table Ts[b]. */
			bkt = mhtbl_getbucket(rows, F, Ts[b], Tsz, sigvtbl, sigv, mhsig, Hs);
			
			/* Initialize bucket, if necessary. */
			if(bkt->_sig == NULL) {
				bkt->_cnt = 0;  /* start with nothing in this bucket */
				bkt->_sz = __MHASHTBL_BKT_DEFAULT_SZ_;  /* default bucket size */
				bkt->_sig = (uint32_t*) malloc(rows * sizeof(uint32_t));
				if(bkt->_sig == NULL)
					return __MHASHTBL_ERR_NO_ALLOCATE_;
				/* Copy signature into new bucket, will act as bucket identifier */
				memcpy(bkt->_sig, mhsig, rows * sizeof(uint32_t));
				bkt->_obvs = (uint32_t*) malloc(bkt->_sz * sizeof(uint32_t));
				if(bkt->_obvs == NULL)
					return __MHASHTBL_ERR_NO_ALLOCATE_;
			}
			
			/* Add more room to bucket, if necessary. This is done by doubling
			 * the size of the bucket. */
			if(bkt->_cnt == bkt->_sz) {  /* expand array, if necessary. */
				/* Allocate new observation index array. */
				bktobvs = (uint32_t*) malloc(bkt->_sz * 2 * sizeof(uint32_t));
				if(bktobvs == NULL)
					return __MHASHTBL_ERR_NO_ALLOCATE_;
				/* Copy old array into new one, free old one. */
				memcpy(bktobvs, bkt->_obvs, bkt->_sz * sizeof(uint32_t));
				free(bkt->_obvs);
				bkt->_obvs = bktobvs;
				/* Update table size. */
				bkt->_sz *= 2;
			}
			bkt->_obvs[bkt->_cnt++] = i;
		}
		
		/* Update signature version, reset table if needed. This will happen
		 * only for very large datasets. Yields amortized O(m) feature lookup 
		 * time. */
		sigv += 1;
		if(sigv == 0) {
			memset(sigvtbl, 0, F * sizeof(uint32_t));
			sigv = 1;
		}
	}
	
	/* Clean up! */
	free(sigvtbl);
	free(mhsig);
	
	/* Throw everything into the provided struct. */
	mht->_B = bands;
	mht->_R = rows;
	mht->_refdb = db;
	mht->_Ts = Ts;
	mht->_Hs = Hs;
	mht->_Tsz = Tsz;
	
	/* Done! */
	return __MHASHTBL_ERR_SUCCESS_;
}

typedef struct {
	uint32_t           _tid;  /* thread identification number */
	uint32_t     _min, _max;  /* range of queries to compute, all q : [min,max) */
	obvdb*             _qdb;  /* query observation database */
	mhtbl*             _mht;  /* minhash table to be queried */
	pthread_mutex_t* _prntr;  /* lock to use file output stream */
	FILE*          _outfile;  /* file to print results to */
	uint32_t      _exclmode;  /* whether or not to include matches to observations with the same index */
	uint32_t       _noprint;  /* whether or not to print the final pairs to stdout */
	uint32_t       _verbose;  /* whether or not to print debugging information */
} mhthrd;

void* mhtbl_query_worker(void* _args) {
	mhthrd* args;
	
	/* Local copy of thread arguments */
	uint32_t           min;
	uint32_t           max;
	obvdb*             qdb;
	mhtbl*             mht;
	pthread_mutex_t* prntr;
	FILE*          outfile;
	uint32_t      exclmode;
	uint32_t       noprint;
	
	/* Meta and misc */
	uint32_t      progress;  /* tracks progress to print to stdout */
	uint32_t i, j, b, r, f;
	
	/* Local copy of minhash table data */
	uint32_t rows, bands;  /* number of rows and bands */
	uint32_t*         Hs;  /* B hash functions */
	_bkt**            Ts;  /* B minhash tables */
	uint32_t         Tsz;  /* size of each of the b minhash tables */
	
	/* Local copy of query observation database data */
	uint32_t        F;  /* dimensionality of observations */
	
	/* Odds and ends */
	uint32_t* sigvtbl;  /* signature version table, permits fast feature
	                       lookup and invalidation. */
	uint32_t     sigv;  /* current version of signature version table */
	uint32_t*   mhsig;  /* stores computed minhash signatures (one at a time) */
	uint32_t*     obv;  /* current observation */
	uint32_t      osz;  /* size of current observation */
	_bkt*         bkt;  /* current bucket */
	uint32_t   bktcnt;  /* number of observations in the current bucket */
	uint32_t*   temps;  /* temporary list to use for reallocation of cands */
	uint32_t*   cands;  /* list of candidate observations */
	uint32_t      csz;  /* size of candidate list */
	uint32_t     ccnt;  /* number of candidate observations */
	uint32_t lastcand;  /* used for eliminating duplicates when printing */
	
	/* Local copy of thread arguments */
	args = (mhthrd*) _args;
	min = args->_min;
	max = args->_max;
	qdb = args->_qdb;
	mht = args->_mht;
	prntr = args->_prntr;
	outfile = args->_outfile;
	exclmode = args->_exclmode;
	noprint = args->_noprint;
	
	/* Meta and misc */
	progress = 0;
	
	/* Local copy of minhash table data */
	rows = mht->_R;
	bands = mht->_B;
	Hs = mht->_Hs;
	Ts = mht->_Ts;
	Tsz = mht->_Tsz;
	
	/* Local copy of query observation database data */
	F = qdb->_F;
	
	/* Allocate and initialize signature version table. */
	sigvtbl = (uint32_t*) calloc(F, sizeof(uint32_t));
	if(sigvtbl == NULL)
		exit(__MHASHTBL_ERR_NO_ALLOCATE_);
	sigv = 1;  /* table initialized to 0, need first version to be non-zero. */
	
	/* Allocate space to store computed minhash signatures. */
	mhsig = (uint32_t*) malloc(rows * sizeof(uint32_t));
	if(mhsig == NULL)
		exit(__MHASHTBL_ERR_NO_ALLOCATE_);
	
	/* Allocate and initialize candidate observation list. */
	csz = 16;  /* initial default size */
	ccnt = 0;  /* no candidates initially */
	cands = (uint32_t*) malloc(csz * sizeof(uint32_t));
	if(cands == NULL)
		exit(__MHASHTBL_ERR_NO_ALLOCATE_);
	
	/* Vacation's over! Thread starts actual work! */
	for(i = min; i < max; i++) {
		/* Build lookup table to perform fast feature membership test. */
		for(j = 0, obv = obvdb_getobv(qdb, i), osz = obvdb_obvsz(qdb, i); j < osz; j++)
			sigvtbl[obv[j]] = sigv;
		
		/* Hash observation i to all B minhash tables. */
		for(b = 0; b < bands; b++) {
			/* Identify bucket this band of the observation maps to. */
			bkt = mhtbl_getbucket(rows, F, Ts[b], Tsz, sigvtbl, sigv, mhsig, Hs);
			
			/* If observation band maps to a bucket, merge observations with 
			 * existing candidates. Expand existing candidate list, if 
			 * necessary.*/
			if(bkt->_obvs != NULL) {
				bktcnt = bkt->_cnt;
				
				/* Expand candidate array to fit new observations. */
				if(bktcnt+ccnt >= csz) {
					csz = (ccnt + bktcnt) * 2;  /* guarantees new obvs will fit plus some */
					temps = (uint32_t*) malloc(csz * sizeof(uint32_t));
					if(temps == NULL)
						exit(__MHASHTBL_ERR_NO_ALLOCATE_);
					memcpy(temps, cands, ccnt * sizeof(uint32_t));
					free(cands);
					cands = temps;
				}
				
				/* Copy bucket observations into candidate array. */
				for(j = 0; j < bktcnt; j++)
					cands[j+ccnt] = bkt->_obvs[j];
				ccnt += bktcnt;
			}
		}
		
		/* Sort candidate observations. */
		qsort(cands, ccnt, sizeof(uint32_t), compare);
		
		/* Print out final results. */
		if(!noprint) {
			pthread_mutex_lock(prntr);
			fprintf(outfile, "%d", i);
			if(ccnt > 0) {
				lastcand = -1;
				if(cands[0] != i || exclmode == 0)
					fprintf(outfile, ",%d", (lastcand = cands[0]));
				for(j = 1; j < ccnt; j++) {
					if(cands[j] != lastcand && (cands[j] != i || exclmode == 0))
						fprintf(outfile, ",%d", cands[j]);
					lastcand = cands[j];
				}
			}
			fprintf(outfile, "\n");
			pthread_mutex_unlock(prntr);
		}else{
			if(args->_verbose) {
				uint32_t diff = max-min+1;
				if(((i-min)*100) / diff > progress) {
					printf("[INFO]    Thread %d: %d%%\n", args->_tid, ++progress);
				}
			}
		}
		
		/* Reset candidate observation list. */
		ccnt = 0;
		
		/* Update signature version, reset table if needed. */
		sigv += 1;  /* invalidates previous observation features */
		if(sigv == 0) {
			memset(sigvtbl, 0, F * sizeof(uint32_t));
			sigv = 1;
		}
	}
	
	/* Clean up! */
	free(sigvtbl);
	free(mhsig);
	free(cands);
	
	/* Done! */
	pthread_exit(NULL);
}

/** Queries the given minhash table with the observations in
 *  the given database. Identified pairs are then outputted to
 *  the specified file. */
uint32_t mhtbl_query(mhtbl* mht, obvdb* qdb, uint32_t thrdcnt, FILE* outfile, uint32_t exclmode, uint32_t noprint, uint32_t verbose) {
	
	uint32_t i, t, rc;
	
	pthread_t*       thrds;  /* array of threads */
	pthread_mutex_t* prntr;  /* lock to use file output stream */
	
	uint32_t      per_thrd;  /* amount of work per thread */
	uint32_t thrd_overflow;  /* number of threads with one more unit of work */
	mhthrd*        mhthrds;  /* per-thread workload assignments */
	
	/* Compute thread work distribution */
	per_thrd = qdb->_N / thrdcnt;
	thrd_overflow = qdb->_N % thrdcnt;
	
	/* Allocate threads */
	thrds = (pthread_t*) malloc(thrdcnt * sizeof(pthread_t));
	if(thrds == NULL)
		return __MHASHTBL_ERR_NO_ALLOCATE_;
	prntr = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
	mhthrds = (mhthrd*) malloc(thrdcnt * sizeof(mhthrd));
	
	/* Initialize threads */
	pthread_mutex_init(prntr, NULL);  /* use default mutex attribs */
	i = 0;
	for(t = 0; t < thrdcnt; t++) {		
		/* Initialize thread arguments */
		mhthrds[t]._tid = t;
		mhthrds[t]._min = i;
		mhthrds[t]._max = (i += (per_thrd + ((t < thrd_overflow) ? 1 : 0)));
		mhthrds[t]._qdb = qdb;
		mhthrds[t]._mht = mht;
		mhthrds[t]._prntr = prntr;
		mhthrds[t]._outfile = outfile;
		mhthrds[t]._exclmode = exclmode;
		mhthrds[t]._noprint = noprint;
		mhthrds[t]._verbose = verbose;
		
		/* Create and run thread */
		pthread_create(&thrds[t], NULL, mhtbl_query_worker, (void*)&mhthrds[t]);
	}
	
	/* Wait for threads to terminate */
	for(t = 0; t < thrdcnt; t++) {
		rc = pthread_join(thrds[t], NULL);
		if(rc != 0)
			return __MHASHTBL_ERR_THRD_JOIN_;
	}
	
	/* Clean up! */
	free(thrds);
	free(mhthrds);
	free(prntr);
	
	/* Done! */
	return __MHASHTBL_ERR_SUCCESS_;
}

/* Returns the bucket that the given observation maps to. The observation data
 * must be stored in the signature version table. Will return NULL if no bucket
 * exists which maps to the computed minhash signature. */
_bkt* mhtbl_getbucket(uint32_t rows, uint32_t F, _bkt* T, uint32_t Tsz, uint32_t* sigvtbl, uint32_t sigv, uint32_t* mhsig, uint32_t* H) {
	uint32_t r, f, hash, bkti;
	
	/* Compute minhash signature for obv_i using H_r's  composing band b */
	for(r = 0; r < rows; r++) {  /* evaluate H_r(obv_i) for all r */
		for(f = 0; f < F; f++) {  /* find first feature in H_r also in obv_i */
			if(sigvtbl[H[f]] == sigv) {
				mhsig[r] = H[f];
				break;
			}
		}
		H += F;
	}
	
	/* Compute hash of minhash signature */
	hash = mhtbl_hash(mhsig, rows);
	
	/* Identify bucket in T_b to insert observation */
	bkti = hash & (Tsz-1);
	while(T[bkti]._sig != NULL && !mhtbl_compare(rows, mhsig, T[bkti]._sig))
		bkti = ((bkti + 1) & (Tsz - 1));
	return T + bkti;
}

/* Compares two minhash signatures, used for comparing signatures when 
 * inserting/getting obvs from mhtbls. Requires O(R) time, where R is the 
 * number of rows per band. */
uint32_t mhtbl_compare(uint32_t rows, uint32_t* mhs1, uint32_t* mhs2) {
	uint32_t i;
	for(i = 0; i < rows; i++)
		if(mhs1[i] != mhs2[i])
			return 0;
	return 1;
}

/* Computes the hash of a given minhash signature, same as Java hashCode(). */
uint32_t mhtbl_hash(uint32_t* mhsig, uint32_t rows) {
	uint32_t hash, r;
	hash = 7;
	for(r = 0; r < rows; r++)
		hash = (hash << 5) - hash + mhsig[r];
	return hash;
}

/* Frees all memory used by the given minhash table. */
void mhtbl_destroy(mhtbl* mht) {
	
	uint32_t i, j;
	
	/* Free used buckets */
	for(i = 0; i < mht->_B; i++) {
		for(j = 0; j < mht->_Tsz; j++) {
			if(mht->_Ts[i][j]._sig) {
				free(mht->_Ts[i][j]._sig);
				free(mht->_Ts[i][j]._obvs);
			}
		}
		free(mht->_Ts[i]);
	}
	
	/* Free tables and hash functions */
	free(mht->_Ts);
	free(mht->_Hs);
	
}
