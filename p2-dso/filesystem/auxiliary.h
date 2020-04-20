
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
#define MAX_NUM_DATABLOCKS 251
//Max disk size = 600 KB --> max number of data blocks = 300 - 1 - 48 = 251

//#endif

