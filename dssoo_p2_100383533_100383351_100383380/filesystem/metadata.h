
/*
 *
 * Operating System Design / Diseño de Sistemas Operativos
 * (c) ARCOS.INF.UC3M.ES
 *
 * @file  metadata.h
 * @brief   Definition of the structures and data types of the file system.
 * @date  Last revision 01/04/2020
 *
 */

#define T_FILE 1
#define T_LINK 2
#define BLOCK_SIZE 2048

#define bitmap_getbit(bitmap_, i_) (bitmap_[i_ >> 3] & (1 << (i_ & 0x07)))
static inline void bitmap_setbit(char *bitmap_, int i_, int val_) {
  if (val_)
    bitmap_[(i_ >> 3)] |= (1 << (i_ & 0x07));
  else
    bitmap_[(i_ >> 3)] &= ~(1 << (i_ & 0x07));
}

typedef struct {    
  unsigned int magicNumber;                 /* Supeblock magic number: 0x000D5500 */

  //these are not used since the maps are selfcontained in the superblock
  //unsigned int numBlocksInodeMap;    /* Number of blocks of the inode map*/
  //unsigned int numBlocksBlockMap;      /* Number of blocks of the data map */   

  unsigned int numInodes;                  /* Number of inodes on the device */    
  unsigned int firstInode;                 /* Block number of of the first inode on the device (root inode) */    
  unsigned int numDataBlocks;               /* Number of data blocks on the device */    
  unsigned int firstDataBlock;             /* Block number of the first block*/    
  unsigned int deviceSize;                 /* Total device size in bytes*/

  char imap[MAX_iNODE_NUM];
  char datablockmap[MAX_NUM_DATABLOCKS];
  
  
  char padding[1679];                 /* Padding for filling a block */
  //Padding = BLOCK_SIZE - superblock size = 2048 - (6*4 + 48+ 297) = 1679
} SuperblockType;

//SuperblockType sBlocks [1] ;
SuperblockType sBlock;

/* No longer needed
int PADDING_I = 1980;
int PADDING_D = 992; 
int numInodes = MAX_iNODE_NUM;
int numDataBlocks = ;
*/

typedef struct {
    unsigned int type;              /*  T_FILE o T_LINK */
    char name[32];              /* name of the associated file/directory*/
    unsigned int inodeTable[5];    /* type==dir: list of files from the directory */
    /*Max file size is 10KB. Block size is 2KB. At most a file will point to 5 different blocks*/
    unsigned int size;              /* File size in bytes */
    unsigned int numBlocks;              /* Number of data blocks used at the moment */
    unsigned int pointsTo;        /*If its a link, what inode should it point to*/
    // 4*4 + 32 + (5*4) = 68 Bytes per iNode.
    //padding not needed with the inode block array
    //char padding[1980];
    // Padding = BLOCK_SIZE - iNode size = 2048 - 68 = 1980
    
} InodeDiskType;

typedef struct {
  InodeDiskType inodeList [24];
  char padding[416];
  //padding = block_size - array_size = 2048 - 68*24 = 416 bytes
} InodeBlockArray;

//InodeDiskType inodos [MAX_iNODE_NUM];
InodeBlockArray inodosBlock [MAX_iNODE_NUM/iNODES_PER_BLOCK];

typedef struct {
	int position;     // Seek position
	int opened;       // 0 if closed, 1 if opened
  int actualBlock;   // Current block
  int integrity;    // 0 if opened without integrity, 1 otherwise
  int crc32_value;  // CRC-32 integrity value
} inode_x;

inode_x file_List [MAX_iNODE_NUM];

//No longer needed. Kept in the superblock
//char imap [numInodes + PADDING_I] ;                 // [BLOCK_SIZE*x]
//char bmap [numDataBlocks + PADDING_D] ;    // [BLOCK_SIZE*y]