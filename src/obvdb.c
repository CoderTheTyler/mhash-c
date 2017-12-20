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
	
	uint32_t f;
	uint64_t i, j;
	
	char* ln;
	char* tkn;
	
	uint64_t    N;
	uint32_t    F;
	uint64_t    M;
	uint32_t* dat;
	uint64_t* szs;
	
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
	F = atoi(tkn);
	
	/* Initialize arrays */
	dat = (uint32_t*) malloc(M * sizeof(uint32_t));
	if(dat == NULL)
		return __OBVDB_INIT_NO_ALLOCATE_;
	szs = (uint64_t*) malloc(N * sizeof(uint64_t));
	if(szs == NULL)
		return __OBVDB_INIT_NO_ALLOCATE_;
	
	/* Parse class body */
	for(i = 0, j = 0; i < N; i++) {
		if(fgets(ln, __OBVDB_BUFFER_SIZE_, src) == NULL)
			return __OBVDB_INIT_MALFORMED_;
		tkn = strtok(ln, ",");
		szs[i] = atoi(tkn);
		for(f = 0; f < szs[i]; f++) {
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
	db->_szs = szs;
	
	/* Done! */
	return __OBVDB_INIT_SUCCESS_;
}

/* Frees memory being used by this database */
void obvdb_destroy(obvdb* db) {
	free(db->_dat);
	free(db->_szs);
}

/* Number of features in observation i */
uint32_t obvdb_sz(obvdb* db, uint64_t i) {
	return db->_dat[db->_szs[i]];
}

/* Pointer to data array starting at observation i */
uint32_t* obvdb_get(obvdb* db, uint64_t i) {
	return db->_dat + db->_szs[i];
}

/* Number of observations in this class */
uint64_t obvdb_cnt(obvdb* db) {
	return db->_N;
}

/* Dimensionality of observations in the database */
uint32_t obvdb_dimensionality(obvdb* db) {
	return db->_F;
}
