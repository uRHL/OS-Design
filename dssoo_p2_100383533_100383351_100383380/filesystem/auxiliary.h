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

/*
 * @brief 	Searchs the i-node with name fname
 * @return 	0 if success, -1 otherwise.
 */
int namei(char *fname);

/*
 * @brief 	Allocates a free i-node
 * @return 	i-node if success, -1 otherwise.
 */
int ialloc(void);

/*
 * @brief 	Free an allocated i-node
 * @return 	0 if success, -1 otherwise.
 */
int ifree (int inode_id);

/*
 * @brief 	Allocates a free block
 * @return 	block id if success, -1 otherwise.
 */
int alloc(void);

/*
 * @brief 	Free an allocated block
 * @return 	0 if success, -1 otherwise.
 */
int freeBlock(int block_id);

/*
 * @brief 	Search block with position equal to offset of i-node inodo_id
 * @return 	block id if success, -1 otherwise.
 */
int bmap(int inodo_id, int offset);