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
#define MAX_NAME_LENGTH 32 //max length for a file name
#define MAX_iNODE_NUM 48
#define iNODES_PER_BLOCK 24
#define MAX_NUM_DATABLOCKS 297
//max numDataBlocks = totalBlocks - superblock - iNode blocks = (max_disk_size / block_size) - 3
// max number of USED data blocks = 48 files * 10kb per file *1024 /2048 bytes per block = 240 blocks


//#endif

/*
 * @brief 	Syncronizes local filesytem with disk
 * @return 	0 if success, -1 otherwise.
 */
int synchronizeWithDisk(void);

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

/*
 * @brief 	Checks that all the constants are well defined, according to the Non functional requirements
 * @return 	0 if no errors, 1 if the NTF1 is not met, 2 if NTF2 is not met, 3 if NTF3 is not met
 , 4 if NTF4 is not met.
 */
int checkFSvalues();

/*
 * @brief 	Close all the files (forced) currently opened. Used when unmounting the file system
 * @return 	0 if all the files could be closed, -1 otherwise
 */
int closeAllFiles();
