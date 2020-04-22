
/*
 *
 * Operating System Design / Diseño de Sistemas Operativos
 * (c) ARCOS.INF.UC3M.ES
 *
 * @file 	filesystem.c
 * @brief 	Implementation of the core file system funcionalities and auxiliary functions.
 * @date	Last revision 01/04/2020
 *
 */


#include "filesystem/filesystem.h" // Headers for the core functionality
#include "filesystem/auxiliary.h"  // Headers for auxiliary functions
#include "filesystem/metadata.h"   // Type and structure declaration of the file system
#include <string.h> //memset function

//SuperblockType sBlock;

/*
 * @brief 	Allocates a free i-node
 * @return 	inode if success, -1 otherwise.
 */
int ialloc(void)
{
	// Search for a free i-node
	for (int i = 0; i < sBlock.numInodes; i++) {
		if (sBlock.imap[i] == 0) {
			// i-node busy right now
			sBlock.imap[i] = 1;

			// Default values for the i-node
			memset(&(inodos[i]), 0, sizeof(InodeDiskType));

			// Return the i-node indentification
			return i;
		}
	}

	return -1;
}


/*
 * @brief 	Free an allocated i-node
 * @return 	0 if success, -1 otherwise.
 */
int ifree(int inode_id)
{
	// Check the inode_id vality, return -1 if not valid
	if (inode_id > sBlock.numInodes) {
		return -1;
	}

	// Free i-node
	sBlock.imap[inode_id] = 0;

	return 0; 
}

/*
 * @brief 	Allocates a free block
 * @return 	block id if success, -1 otherwise.
 */
int alloc(void)
{
	char b[BLOCK_SIZE];
	for (int i = 0; i < sBlock.numDataBlocks; i++) {
		if (sBlock.datablockmap[i] == 0) {
			// Busy block right now
			sBlock.datablockmap[i] = 1;

			// Default values for the block
			memset(b, 0, BLOCK_SIZE);
			bwrite(DEVICE_IMAGE, i, b);

			// Return the block id
			return i;
		}
	}

	return -1;
}

/*
 * @brief 	Free an allocated block
 * @return 	0 if success, -1 otherwise.
 */
int freeBlock(int block_id)
{
	// Check the block_id vality, return -1 if not valid
	if (block_id > sBlock.numDataBlocks) {
		return -1;
	}

	// Free block
	sBlock.datablockmap[block_id] = 0;

	return 0;
}


/*
 * @brief 	Searchs the i-node with name fname
 * @return 	i-node if success, -1 otherwise.
 */
int namei(char *fname)
{
	// If i-node with name equal to fname, return i-node
	for (int i = 0; i < sBlock.numDataBlocks; i++){
 		if (strcmp(inodos[i].name, fname) == 0) {
 			return i;
		}
 	}
	
	return -1;
}

/*
 * @brief 	Search block with position equal to offset of i-node inodo_id
 * @return 	block id if success, -1 otherwise.
 */
int bmap(int inodo_id, int offset)
{
	int b[BLOCK_SIZE / 4];

	// If offset is lower than the block size, return the direct block
	if (offset < BLOCK_SIZE) {
		return inodos[inodo_id].FirstBlock;
	}
	
	// Otherwise, it means there are indirect blocks. Therefore, return indirect block
	if (offset < BLOCK_SIZE * BLOCK_SIZE / 4) {
		if (bread(DEVICE_IMAGE, inodos[inodo_id].indirectBlock, b) < 0) {
			printf("Error! Block starting at position %d of i-node %d couldn't be read.\n", offset, inodo_id);
			return -1;
		}

		offset = (offset – BLOCK_SIZE) / BLOCK_SIZE;

		return b[offset];
	}

	// If error return -1
	return -1; 
}

/*
 * @brief 	Generates the proper file system structure in a storage device, as designed by the student.
 * @return 	0 if success, -1 otherwise.
 */
int mkFS(long deviceSize)
{
	if(deviceSize < MIN_DISK_SIZE || deviceSize > MAX_DISK_SIZE){
		printf("Error! Disk size should be between 460KB and 600KB.\n");
		return -1;
	}

	// Setup with default values the superblock, maps, and i-nodes	 
	sBlock.magicNumber = 0x000D5500;   
	sBlock.numInodes = MAX_iNODE_NUM;		
    sBlock.numDataBlocks = MAX_NUM_DATABLOCKS;

	// Not needed. See metadata.h
    //sBlock.numBlocksInodeMap = ; //Cuantos bloques necesito para mapear todos los nodos (todos los punteros)
    //sBlock.numBlocksBlockMap = 48; //Cuantos bloques necesito para mapear todos los data blocks (todos los punteros)

	sBlock.firstInode = 1; //Superblock ocupies 1
		
	// num Inode blocks + num superblock = 49
	sBlock.firstDataBlock = 49; //superblock in 0 plus 48 inodes
	sBlock.deviceSize = deviceSize;

	// Free i-map
	for (int i = 0; i < sBlock.numInodes; i++){           
	    sBlock.imap[i] = 0;
	}

	// Free block map
	for (int i = 0; i < MAX_NUM_DATABLOCKS; i++){
		sBlock.datablockmap[i] = 0;
	}

	for (int i = 0; i < sBlock.numInodes; i++){
	    memset(&(inodos[i]), 0, sizeof(InodeDiskType) );
	} 

	// Synchronize disk
	if (syncronizeWithDisk() < 0) {
		printf("Error! Couldn't synchronize with disk.\n");
		return -1;
	}

	/*// We also prepare the file array
	for(int i=0;i<sBlock.numInodes;i++){
		file_List[i].position=0;
		file_List[i].opened=0;
	}
*/
	printf("New file system made.\n");
	return 0;
}

/*
 * @brief 	Syncronizes local filesytem with disk
 * @return 	0 if success, -1 otherwise.
 */
int syncronizeWithDisk()
{
	// Write block 0 from sBlock into disk, if error return -1
	if (bwrite(DEVICE_IMAGE, 0, (char *)&(sBlock)) < 0) {
		printf("Error! Cannot write block 0 from sBlock into disk.\n");
		return -1;
	}

	// Write the i-nodes to disk, if error return -1
	// We start writing in block 1 because all the previous information is stored in the superblock
	for (int i = 0; i < (sBlock.numInodes * sizeof(InodeDiskType) / BLOCK_SIZE); i++){
		if (bwrite(DEVICE_IMAGE, 1 + i, ((char *)inodos + i * BLOCK_SIZE)) < 0) {
			printf("Error! Cannot write block %d from sBlock into disk.\n", i);
			return -1;
		}
	}

	return 0;
}


/*
 * @brief 	Mounts a file system in the simulated device.
 * @return 	0 if success, -1 otherwise.
 */
int mountFS(void)
{
	// Write block 0 from sBlock into disk    
	if (bread(DEVICE_IMAGE, 0, (char *)&(sBlock)) < 0) {
		printf("Error! Cannot write block 0 from disk into sBlock.\n");
		return -1;
	}

	// Write i-nodes to disk
	for (int i = 0; i < (sBlock.numInodes * sizeof(InodeDiskType) / BLOCK_SIZE); i++){
		if (bread(DEVICE_IMAGE, i + 1, ((char *)inodos + i * BLOCK_SIZE))) {
			printf("Error! Cannot write block %d from disk into sBlock.\n", i);
			return -1;
		}
	}

	printf("File system mounted.\n");
    return 0;
}

/*
 * @brief 	Unmounts the file system from the simulated device.
 * @return 	0 if success, -1 otherwise.
 */
int unmountFS(void)
{
	// Check if opened files
	for (int i = 0; i < MAX_iNODE_NUM; i++) {
		if (file_List[i].opened == 1){
			printf("There are files that haven't been closed\n");
			return -1;
		}
	}

	// Synchronize disk
	if (syncronizeWithDisk() < 0) {
		printf("Error! Couldn't synchronize disk.\n");
		return -1;
	}

	printf("File system unmounted.\n");
	return 0;
}

/*
 * @brief	Creates a new file, provided it it doesn't exist in the file system.
 * @return	0 if success, -1 if the file already exists, -2 in case of error.
 */
int createFile(char *fileName)
{	
	// Error if the file already exists
	if (namei(fileName) != 0) {
		printf("The file with name %s already exists in the file system.\n", fileName);
		return -1;
	}

	// Block and i-node IDs
	int block_id, inode_id;

	// Allocate a new i-node
	inode_id = ialloc();

	// If error return -1
	if (inode_id < 0) {
		printf("Error! i-node of file %s couldn't be allocated.\n", fileName);
		return -2;
	}

	// Allocate a new block
	block_id = alloc();

	// If error free i-node and return -2
	if (block_id < 0) {
		printf("Error! Block of file %s couldn't be allocated.\n", fileName);
		ifree(inode_id);
		return -2;
	}

	// Set file values
	inodos[inode_id].type = T_FILE ;
	strcpy(inodos[inode_id].name, fileName);
	inodos[inode_id].FirstBlock = block_id ;
	file_List[inode_id].position = 0;
	file_List[inode_id].opened = 1;

	printf("File %s created.\n", fileName);
	return 0; 
}

/*
 * @brief	Deletes a file, provided it exists in the file system.
 * @return	0 if success, -1 if the file does not exist, -2 in case of error..
 */
int removeFile(char *fileName)
{
	int inode_id = namei(fileName);

	// Error if the file doesn't exist
	if (inode_id < 0) {
		printf("The file %s does not exists.\n", fileName);
		return -1;
	}

	// Free block, if error return -2
	if (freeBlock(inodos[inode_id].FirstBlock) < 0) {
		printf("Error! Block of file %s couldn't be freed.\n", fileName);
		return -2;
	}
	memset(&(inodos[inode_id]), 0, sizeof(InodeDiskType));

	// Free i-node, if error return -2
	if (ifree(inode_id) < 0) {
		printf("Error! i-node of file %s couldn't be freed.\n", fileName);
		return -2;
	}

	printf("File %s removed.\n", fileName);
	return 0;
}

/*
 * @brief	Opens an existing file.
 * @return	The file descriptor if possible, -1 if file does not exist, -2 in case of error..
 */
int openFile(char *fileName)
{  
	//First of all we look if the file exist, if we didn't find we return -1
	int inode_id = namei(fileName);

	// If error return -1
	if (inode_id < 0) {
		printf("Error! File %s doesn't exist.\n", fileName);
		return -1 ;
	}
    
	file_List[inode_id].position = 0;	// Seek position = 0
	file_List[inode_id].opened = 1;		// Open bit = 1

	printf("File %s opened.\n", fileName);
	return inode_id; 
}

/*
 * @brief	Closes a file.
 * @return	0 if success, -1 otherwise.
 */
int closeFile(int fileDescriptor)
{
	if (fileDescriptor < 0){
		printf("Error! Wrong file descriptor.\n");
		return -1;
	}

	//We look in the inode map if that file has been created. 
	if (!bitmap_getbit(sBlock.imap, fileDescriptor)){
		printf("Error! There is no file with such file descriptor.\n");
		return -1;
	}

	file_List[fileDescriptor].position = 0;		// Seek position = 0
	file_List[fileDescriptor].opened = 0;		// Open bit = 0

	printf("File with file descriptor %d closed.\n", fileDescriptor);
	return 0;
}

/*
 * @brief	Reads a number of bytes from a file and stores them in a buffer.
 * @return	Number of bytes properly read, -1 in case of error.
 */
int readFile(int fileDescriptor, void *buffer, int numBytes)
{
	char b[BLOCK_SIZE];
	int block_id;

	// If file position plus number of bytes to be read is greater that the file size,
	// the number of bytes to be read is resized to what's left of the file
	if (file_List[fileDescriptor].position + numBytes > inodos[fileDescriptor].size) {
		numBytes = inodos[fileDescriptor].size - file_List[fileDescriptor].position;
	}

	// If error return -1
	if (numBytes <= 0) {
		printf("Error! File with id %d couldn't be read.\n", fileDescriptor);
		return -1;
	}

	block_id = bmap(fileDescriptor, file_List[fileDescriptor].position);

	if (block_id < 0) {
		printf("Error! Block of file with id %d couldn't be read.\n", fileDescriptor);
		return -1;
	}

	if (bread(DEVICE_IMAGE, block_id, b) < 0) {
		printf("Error! Block %d of file with id %d couldn't be read.\n", block_id, fileDescriptor);
		return -1;
	}

	// Save content to buffer
	memmove(buffer, b + file_List[fileDescriptor].position, numBytes);

	// Increase file position
	file_List[fileDescriptor].position += numBytes;

	return numBytes;
}

/*
 * @brief	Writes a number of bytes from a buffer and into a file.
 * @return	Number of bytes properly written, -1 in case of error.
 */
int writeFile(int fileDescriptor, void *buffer, int numBytes)
{
	char b[BLOCK_SIZE];
	int block_id;

	// If file position plus number of bytes to be written is greater that the file size,
	// the number of bytes to be written is resized to what's left of the file
	if (file_List[fileDescriptor].position + numBytes > BLOCK_SIZE) {
		numBytes = BLOCK_SIZE - file_List[fileDescriptor].position;
	}

	// If error return -1
	if (numBytes <= 0) {
		printf("Error! File with id %d couldn't be written.\n", fileDescriptor);
		return -1;
	}

	block_id = bmap(fd, file_List[fileDescriptor].position);
	if (bread(DEVICE_IMAGE, block_id, b) < 0) {
		printf("Error! Block %d of file with id %d couldn't be read.\n", block_id, fileDescriptor);
		return -1;
	}

	// Write content to file
	memmove(b + file_List[fileDescriptor].position, buffer, numBytes);
	bwrite(DEVICE_IMAGE, block_id, b);

	// Increase file position
	fileDescriptor[fileDescriptor].position += numBytes;

	// Increase file size
	inodos[fileDescriptor].size += numBytes;

	return numBytes; 
}

/*
 * @brief	Modifies the position of the seek pointer of a file.
 * @return	0 if success, -1 otherwise.
 */
int lseekFile(int fileDescriptor, long offset, int whence)
{	
	// If error return -1
	if (fileDescriptor < 0) {
		printf("Error! Wrong file descriptor.\n");
		return -1;
	}

	// Change file position to the beginning
	if (whence == FS_SEEK_BEGIN) {
		file_List[fileDescriptor].position = 0;
	}

	// Change file position to its current position plus an offset
	else if (whence == FS_SEEK_CUR && file_List[fileDescriptor].position + offset <= inodos[fileDescriptor].size) {
		file_List[fileDescriptor].position += offset;
	}

	// Change file position to the end
	else if (whence == FS_SEEK_END) {
		file_List[fileDescriptor].position = inodos[fileDescriptor].size;
	}

	// If error return -1
	else {
		printf("Error! Incorrect whence value.\n");
		return -1;
	}

	return 0;
}

/*
 * @brief	Checks the integrity of the file.
 * @return	0 if success, -1 if the file is corrupted, -2 in case of error.
 */

int checkFile (char * fileName)
{

    return -2;
}

/*
 * @brief	Include integrity on a file.
 * @return	0 if success, -1 if the file does not exists, -2 in case of error.
 */

int includeIntegrity (char * fileName)
{
    return -2;
}

/*
 * @brief	Opens an existing file and checks its integrity
 * @return	The file descriptor if possible, -1 if file does not exist, -2 if the file is corrupted, -3 in case of error
 */
int openFileIntegrity(char *fileName)
{

    return -2;
}

/*
 * @brief	Closes a file and updates its integrity.
 * @return	0 if success, -1 otherwise.
 */
int closeFileIntegrity(int fileDescriptor)
{
    return -1;
}

/*
 * @brief	Creates a symbolic link to an existing file in the file system.
 * @return	0 if success, -1 if file does not exist, -2 in case of error.
 */
int createLn(char *fileName, char *linkName)
{
    return -1;
}

/*
 * @brief 	Deletes an existing symbolic link
 * @return 	0 if the file is correct, -1 if the symbolic link does not exist, -2 in case of error.
 */
int removeLn(char *linkName)
{
    return -2;
}
