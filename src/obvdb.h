#ifndef __OBVDB_H_
#define __OBVDB_H_

#include <stdint.h>
#include <stdio.h>

typedef struct {
	uint32_t    _F;  /* number of feature dimensions */
	uint32_t    _N;  /* number of observations */
	uint32_t* _dat;  /* features in each observation */
	uint32_t* _szs;  /* prefix sum of sizes of observations */
} obvdb;

/* Initializes the given pointer with a database */
uint32_t obvdb_init(obvdb* db, FILE* src);

/* Frees memory being used by this database */
void obvdb_destroy(obvdb* db);

/* Dimensionality of observations in the database */
uint32_t obvdb_dimensionality(obvdb* db);

/* Number of features in observation i */
uint32_t obvdb_sz(obvdb* db, uint32_t i);

/* Pointer to data array starting at observation i */
uint32_t* obvdb_get(obvdb* db, uint32_t i);

/* Number of observations in this class */
uint32_t obvdb_cnt(obvdb* db);

#endif
