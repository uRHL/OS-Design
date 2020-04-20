
/*
 *
 * Operating System Design / Diseño de Sistemas Operativos
 * (c) ARCOS.INF.UC3M.ES
 *
 * @file 	metadata.h
 * @brief 	Definition of the structures and data types of the file system.
 * @date	Last revision 01/04/2020
 *
 */

#define bitmap_getbit(bitmap_, i_) (bitmap_[i_ >> 3] & (1 << (i_ & 0x07)))
static inline void bitmap_setbit(char *bitmap_, int i_, int val_) {
  if (val_)
    bitmap_[(i_ >> 3)] |= (1 << (i_ & 0x07));
  else
    bitmap_[(i_ >> 3)] &= ~(1 << (i_ & 0x07));
}

typedef struct {    
	unsigned int magicNumber;                 /* Supeblock magic number: 0x000D5500 */
    unsigned int numBlocksInodeMap;    /* Number of blocks of the inode map*/
    unsigned int numBlocksBlockMap;      /* Number of blocks of the data map */   
    unsigned int numInodes;                  /* Number of inodes on the device */    
    unsigned int firstInode;                 /* Block number of of the first inode on the device (root inode) */    
    unsigned int numDataBlocks;               /* Number of data blocks on the device */    
    unsigned int firstDataBlock;             /* Block number of the first block*/    
    unsigned int deviceSize;                 /* Total device size in bytes*/
    char padding[992];                 /* Padding for filling a block */
} SuperblockType;

SuperblockType sBlocks [1] ;
int PADDING_I = 992;
int PADDING_D = 992; 
int numInodes =48;
int numDataBlocks =48;

typedef struct {
    unsigned int type;              /*  T_FILE o T_DIRECTORY */
    char name[32];              /* name of the associated file/directory*/
    unsigned int inodeTable[48];    /* type==dir: list of inodes from the directory */
    unsigned int size;              /* File size in bytes */
    unsigned int directBlock;              /* Direct block number */    
    unsigned int indirectBlock;              /* Indirect block number */    
    char padding[PADDING_I];            /* Padding for filling a block */
} InodeDiskType;

InodeDiskType inodos [48] ;

char imap [numInodes + PADDING_I] ;                 // [BLOCK_SIZE*x]
char bmap [numDataBlocks + PADDING_D] ;    // [BLOCK_SIZE*y]