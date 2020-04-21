
/*
 *
 * Operating System Design / Diseño de Sistemas Operativos
 * (c) ARCOS.INF.UC3M.ES
 *
 * @file 	auxiliary.h
 * @brief 	Headers for the auxiliary functions required by filesystem.c.
 * @date	Last revision 01/04/2020
 *
 */

//#ifndef
#define BLOCK_SIZE 2048
#define MAX_DISK_SIZE 614400 //600 KB
#define MIN_DISK_SIZE 471040 //460 KB
#define MAX_iNODE_NUM 48
#define MAX_NUM_DATABLOCKS 240 // max number of data blocks = 48 files * 10kb per file *1024 /2048 bytes per block = 240 blocks

//#endif

/*
 * @brief 	Syncronizes local filesytem with disk
 * @return 	0 if success, -1 otherwise.
 */
int syncronizeWithDisk(void);
