#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "obvdb.h"

#define __OBVDB_BUFFER_SIZE_      1048576
#define __OBVDB_INIT_SUCCESS_     0
#define __OBVDB_INIT_MALFORMED_   1
#define __OBVDB_INIT_NO_ALLOCATE_ 2

/* Initializes the given pointer with a database */
uint32_t obvdb_init(obvdb* db, FILE* src) {
	
	uint32_t f, m, i, j;
	
	char* ln;
	char* tkn;
	
	uint32_t    N,F;
	uint64_t      M;
	uint32_t*   dat;
	uint64_t* szpfx;
	
	/* Initialize buffer to read file with */
	ln = (char*) malloc(__OBVDB_BUFFER_SIZE_ * sizeof(char));
	if(ln == NULL)
		return __OBVDB_INIT_NO_ALLOCATE_;
	
	/* Read and parse header */
	if(fgets(ln, __OBVDB_BUFFER_SIZE_, src) == NULL)
		return __OBVDB_INIT_MALFORMED_;
	tkn = strtok(ln, ",");
	N = atoll(tkn);
	tkn = strtok(NULL, ",");
	M = atoll(tkn);
	tkn = strtok(NULL, ",");
	F = atoll(tkn);
	
	/* Initialize arrays */
	dat = (uint32_t*) malloc(M * sizeof(uint32_t));
	if(dat == NULL)
		return __OBVDB_INIT_NO_ALLOCATE_;
	szpfx = (uint64_t*) malloc((N+1) * sizeof(uint64_t));
	if(szpfx == NULL)
		return __OBVDB_INIT_NO_ALLOCATE_;
	
	/* Parse class body */
	szpfx[0] = 0;
	for(i = 0, j = 0; i < N; i++) {
		if(fgets(ln, __OBVDB_BUFFER_SIZE_, src) == NULL)
			return __OBVDB_INIT_MALFORMED_;
		tkn = strtok(ln, ",");
		m = atoi(tkn);
		szpfx[i+1] = szpfx[i] + m;  /* m = szpfx[i+1] - szpfx[i] */
		for(f = 0; f < m; f++) {
			tkn = strtok(NULL, ",");
			dat[j++] = atoi(tkn);
		}
	}
	
	/* Clean up! */
	free(ln);
	
	/* Store parsed information in struct */
	db->_N = N;
	db->_F = F;
	db->_dat = dat;
	db->_szpfx = szpfx;
	
	/* Done! */
	return __OBVDB_INIT_SUCCESS_;
}

/* Frees memory being used by this database */
void obvdb_destroy(obvdb* db) {
	free(db->_dat);
	free(db->_szpfx);
}

/* Number of features in observation i */
uint32_t obvdb_obvsz(obvdb* db, uint32_t i) {
	return db->_szpfx[i+1] - db->_szpfx[i];
}

/* Return pointer to i-th observation in this database */
uint32_t* obvdb_getobv(obvdb* db, uint32_t i) {
	return db->_dat + db->_szpfx[i];
}

/* Number of observations in this class */
uint32_t obvdb_cnt(obvdb* db) {
	return db->_N;
}

/* Dimensionality of observations in the database */
uint32_t obvdb_dimensionality(obvdb* db) {
	return db->_F;
}
