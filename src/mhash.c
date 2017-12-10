#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "obvdb.h"
#include "mhtbl.h"

int main(int argc, char* argv[]) {
	
	/* Miscellaneous junk */
	uint32_t i, j;
	
	/* Meta settings */
	uint32_t       seed;
	uint32_t    verbose;
	uint32_t  excl_mode;
	uint32_t thread_cnt;
	
	/* MinHash settings & data */
	uint32_t bands;  /* number of bands (OR, # mhfs) */
	uint32_t  rows;  /* length of each band (AND, mhf size) */
	
	/* Observation data */
	FILE*   infile;
	obvdb* queries;
	obvdb* targets;
	mhtbl*   table;
	
	
	/*********************
	 * Program arguments *
	 *********************/
	
	/* Initialize args to defaults */
	verbose = 0;
	excl_mode = 0;
	thread_cnt = 1;
	seed = 0;
	bands = 0;
	rows = 0;
	
	/* Parse provided arguments */
	for(i = 0; i < (uint32_t)argc; i++) {
		if(strcmp(argv[i],"-b") == 0)
			bands = atoi(argv[++i]);
		else if(strcmp(argv[i],"-r") == 0)
			rows = atoi(argv[++i]);
		else if(strcmp(argv[i],"-t") == 0)
			thread_cnt = atoi(argv[++i]);
		else if(strcmp(argv[i],"-s") == 0)
			seed = atoi(argv[++i]);
		else if(strcmp(argv[i],"-E") == 0)
			excl_mode = 1;
		else if(strcmp(argv[i],"-V") == 0)
			verbose = 1;
	}
	
	/* Verify arguments */
	if(bands == 0) {
		printf("[ARGS]    Number of bands must be a positive integer! See -b flag.\n");
		return -100;
	}
	if(rows == 0) {
		printf("[ARGS]    Number of rows must be a positive integer! See -r flag.\n");
		return -101;
	}
	if(thread_cnt == 0) {
		printf("[ARGS]    Number of threads must be a positive integer! See -T flag.\n");
		return -102;
	}
	
	
	/******************************
	 * Parse target/query classes *
	 ******************************/
	
	/* Determine input file */
	infile = stdin;
	
	/* Initialize query observation database */
	queries = (obvdb*) malloc(sizeof(obvdb));
	if(queries == NULL) {
		printf("[ERROR]   Failed to allocate query observation database!\n");
		return -200;
	}
	if(verbose) 
		printf("[INFO]    Parsing query observation data...\n");
	obvdb_init(queries, infile);
	if(verbose)
		printf("[SUCCESS] Query observation data parsed successfully!\n");
	
	/* Initialize target observation database */
	if(excl_mode == 0) {
		targets = (obvdb*) malloc(sizeof(obvdb));
		if(targets == NULL) {
			printf("[ERROR]   Failed to allocate target observation database!\n");
			return -300;
		}
		if(verbose)
			printf("[INFO]    Parsing target observation data...\n");
		obvdb_init(targets, infile);
		if(verbose)
			printf("[SUCCESS] Target observation data parsed successfully!\n");
	}else
		targets = queries;
	
	/* Verify queries and targets are comparable */
	i = obvdb_dimensionality(queries);
	j = obvdb_dimensionality(targets);
	if(i != i) {
		printf("[ERROR]   Dimensionality mismatch between queries and targets! %d != %d\n", i, j);
		return -301;
	}
	
	
	/*********************
	 * Build hash tables *
	 *********************/
	
	table = (mhtbl*) malloc(sizeof(mhtbl));
	if(table == NULL) {
		printf("[ERROR]   Failed to allocate minhash tables!\n");
		return -400;
	}
	if(mhtbl_init(table, targets, bands, rows, seed) != __MHASHTBL_ERR_SUCCESS_) {
		printf("[ERROR]   Failed to initialize minhash tables!\n");
		return -500;
	}
	
	
	/*****************************
	 * Lookup query observations *
	 *****************************/
	
	if(mhtbl_query(table, queries, thread_cnt, stdout, excl_mode)) {
		printf("[ERROR]   Failed to query minhash tables!\n");
		return -600;
	}
	
	
	/*************
	 * Clean up! *
	 *************/
	
	/* Free observation databases */
	obvdb_destroy(queries);
	free(queries);
	if(excl_mode == 0) {
		obvdb_destroy(targets);
		free(targets);
	}
	
	/* Free minhash table */
	mhtbl_destroy(table);
	free(table);
	
	/* Done! */
	return 0;
}

