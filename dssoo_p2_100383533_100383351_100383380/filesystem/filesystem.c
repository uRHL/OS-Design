
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
#include <stdlib.h> //memory allocation functions

//SuperblockType sBlock;
//Variable to prevent the FS to be mounted before it is allocated in main memory (mkFS())
int FS_ALLOCATED = 0;
//Variable to prevent mount and unmount operations if they are not called in correct order (1.Mount, 2.Unmount)
int FS_MOUNTED = 0;

/*
 * @brief 	Allocates a free i-node
 * @return 	inode if success, -1 otherwise.
 */
int ialloc(void)
{
	// Search for a free i-node
	for (int i = 0; i < MAX_iNODE_NUM; i++) {
		if (sBlock.imap[i] == 0) {
			// i-node busy right now
			sBlock.imap[i] = 1;

			// Default values for the i-node
			//memset(&(inodos[i]), 0, sizeof(InodeDiskType)); //Using 1 inode per block

			//Using inode block array
			memset(&(inodosBlock[i / iNODES_PER_BLOCK].inodeList[i % iNODES_PER_BLOCK]), 0, sizeof(InodeDiskType));

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
	if (inode_id > MAX_iNODE_NUM) {
		return -1;
	}

	// Free i-node
	sBlock.imap[inode_id] = 0;

	return 0; 
}

/*
 * @brief 	Allocates a free block
 * @return 	block id if success [0, numDataBlocks], -1 otherwise.
 */
int alloc(void)
{
	char * b = calloc(BLOCK_SIZE, sizeof(char));
	for (int i = 0; i < sBlock.numDataBlocks; i++) {
		if (sBlock.datablockmap[i] == 0) {
			// Busy block right now
			sBlock.datablockmap[i] = 1;

			// Default values for the block
			memset(b, 0, (size_t)BLOCK_SIZE);
			//i + 3 because the first three blocks of the disk are metadata
			bwrite(DEVICE_IMAGE, i + 3, b);

			// Return the block id
			//The real id used in the device will be i + 3, since the three first blocks are metadata
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
	//Comprobar que pasa con los datos que había en esos bloques
	return 0;
}


/*
 * @brief 	Searchs the i-node with name fname
 * @return 	i-node if success, -1 otherwise.
 */
int namei(char *fname)
{
	// If i-node with name equal to fname, return i-node
	for (int i = 0; i < MAX_iNODE_NUM; i++){
 		if (strcmp(inodosBlock[i / iNODES_PER_BLOCK].inodeList[i % iNODES_PER_BLOCK].name, fname) == 0) {
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
	//Not used. Not implemented
	return -1; 
}


/*
 * @brief 	Checks that all the constants are well defined, according to the Non functional requirements
 * @return 	0 if no errors, 1 if the NTF1 is not met, 2 if NTF2 is not met, 3 if NTF3 is not met
 , 4 if NTF4 is not met.
 */
int checkFSvalues() {
	//Checking NTF1
	if (MAX_iNODE_NUM != 48) return 1;
	//Checking NTF2
	if (MAX_NAME_LENGTH != 32) return 2;
	//Checking NTF3
	if (MAX_FILE_SIZE != (10*1024)) return 3;
	//Checking NTF4
	if (BLOCK_SIZE != 2048) return 4;
	//no errors at this point
	return 0;

}

/*
 * @brief 	Close all the files (forced) currently opened. Used when unmounting the file system
 * @return 	0 if all the files could be closed, -1 otherwise
 */
int closeAllFiles() {
	int aux, ret = 0;
	for (int fd = 0; fd < MAX_iNODE_NUM; fd++){
		//If the file exists and is opened, try to close it
		if(sBlock.imap[fd] && file_List[fd].opened){
			//Check if the file has been opened with integrity
			if(file_List[fd].integrity){
				aux = closeFileIntegrity(fd);
			}else{
				aux = closeFile(fd);
			}
			//The file could not be closed properly
			if(aux == -1){
				ret = -1;
			}
		}
	}
	return ret;

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

	/*If the maximum file size is not a multiple of the block size a decision must be made:
	truncate the size of the file, or extend its size.
	Given that this values are defined by the programmer, this conditional is created as a security measure
	so that the file system will not be created if this two constants are not well defined.
	*/
	if(MAX_FILE_SIZE % BLOCK_SIZE){
		printf("Error! Max_FILE_SIZE is not a multiple of BLOCK_SIZE.\n");
		return -1;
	}
	//Checking that the constants are well defined according to the Non Functional Requirements
	int ret = checkFSvalues();
	if (ret){
		printf("Error! The NTF%d is not met", ret);
		return -1;
	}

	// Setup with default values the superblock, maps, and i-nodes	 
    sBlock.numDataBlocks = (deviceSize/BLOCK_SIZE) - 3;
	// 3 blocks reserved: superblock and inodes

	sBlock.firstInode = 1; //Superblock ocupies the first block of the disk
		
	// num Inode blocks + num superblock = 3
	sBlock.firstDataBlock = 3; //superblock in 0 plus 2 inode blocks
	sBlock.deviceSize = deviceSize;

	// Free i-map
	for (int i = 0; i < MAX_iNODE_NUM; i++){           
	    sBlock.imap[i] = 0;
	}

	// Free block map
	for (int i = 0; i < MAX_NUM_DATABLOCKS; i++){
		if (i < sBlock.numDataBlocks) 
			sBlock.datablockmap[i] = 0;
		else{
			//Marking further blocks as invalid since they exceed the actual number of data blocks for the given disk
			sBlock.datablockmap[i] = -1;
		} 
	}

	for (int i = 0; i < MAX_iNODE_NUM; i++){
		//reserve memory for the inodes	    
		memset(&(inodosBlock[i/iNODES_PER_BLOCK].inodeList[i%iNODES_PER_BLOCK]), 0, sizeof(InodeDiskType));
	} 

	// Synchronize disk
	if (synchronizeWithDisk() < 0) {
		printf("Error! Couldn't synchronize with disk.\n");
		return -1;
	}
	FS_ALLOCATED = 1;
	printf("New file system made.\n");
	return 0;
}

/*
 * @brief 	Syncronizes local filesytem with disk
 * @return 	0 if success, -1 otherwise.
 */
int synchronizeWithDisk()
{
	// Write block sBlock into disk, if error return -1
	if (bwrite(DEVICE_IMAGE, 0, (char *)&(sBlock)) < 0) {
		printf("Error! Cannot write block 0 from sBlock into disk.\n");
		return -1;
	}

	// Write the i-nodes to disk, if error return -1
	// We start writing in block 1 because all the previous information is stored in the superblock
	
	for (int i = 0; i < (MAX_iNODE_NUM / iNODES_PER_BLOCK); i++){
		if (bwrite(DEVICE_IMAGE, 1 + i, (char *)&inodosBlock[i]) < 0) {
			printf("Error! Cannot write block %d from iNodeBlock into disk.\n", i);
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
	//If the FS has not been allocated in main memory it cannot be mounted
	if (!FS_ALLOCATED){
		printf("Error! The file system has not been allocated in main memory. Use mkFS().\n");
		return -1;
	}
	// Write block 0 from sBlock into disk    
	if (bread(DEVICE_IMAGE, 0, (char *)&(sBlock)) < 0) {
		printf("Error! Cannot write block 0 from disk into sBlock.\n");
		return -1;
	}



	// Write i-nodes to disk
	for (int i = 0; i < (MAX_iNODE_NUM / iNODES_PER_BLOCK); i++){
		if (bread(DEVICE_IMAGE, 1 + i, (char *)&inodosBlock[i]) < 0) {
			printf("Error! Cannot write block %d from sBlock into disk.\n", i);
			return -1;
		}
	}
	FS_MOUNTED = 1;
	printf("File system mounted.\n");
    return 0;
}

/*
 * @brief 	Unmounts the file system from the simulated device.
 * @return 	0 if success, -1 otherwise.
 */
int unmountFS(void)
{

	//Check that the FS is mounted
	if(!FS_MOUNTED){
		printf("There is no file system currently mounted\n");
		return -1;
	}
	// Check if opened files
	for (int i = 0; i < MAX_iNODE_NUM; i++) {
		if (sBlock.imap[i] && file_List[i].opened){
			printf("There are files that haven't been closed\n");
			return -1;
		}
	}

	// Synchronize disk
	if (synchronizeWithDisk() < 0) {
		printf("Error! Couldn't synchronize disk.\n");
		return -1;
	}

	FS_MOUNTED = 0;
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
	if (namei(fileName) !=-1) {
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
	//inodos[i] = inodosBlock[i / iNODES_PER_BLOCK].inodeList[i % iNODES_PER_BLOCK]
	
	inodosBlock[inode_id / iNODES_PER_BLOCK].inodeList[inode_id % iNODES_PER_BLOCK].type = T_FILE ;
	strcpy(inodosBlock[inode_id / iNODES_PER_BLOCK].inodeList[inode_id % iNODES_PER_BLOCK].name, fileName);
	inodosBlock[inode_id / iNODES_PER_BLOCK].inodeList[inode_id % iNODES_PER_BLOCK].numBlocks = 1 ;
	inodosBlock[inode_id / iNODES_PER_BLOCK].inodeList[inode_id % iNODES_PER_BLOCK].inodeTable[0] = block_id;
	file_List[inode_id].position = 0;
	file_List[inode_id].opened = 1;
	file_List[inode_id].currentBlock = 0;
	//crc = 0 means that the file do not have integrity
	file_List[inode_id].crc32_value = 0;

	//Update the file metadata
	//The first block of the device is used for the superblock, thus the offset
	if(bwrite(DEVICE_IMAGE, (inode_id / iNODES_PER_BLOCK) + 1, (char *)&(inodosBlock[inode_id / iNODES_PER_BLOCK]))){
		printf("Error writing the file into disk\n");
		return -1;
	}

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

	if(inodosBlock[inode_id / iNODES_PER_BLOCK].inodeList[inode_id % iNODES_PER_BLOCK].type==T_LINK){
		printf("Error! %s is a link, not a file\n", fileName);
		return -2;
	}
	memset(&(inodosBlock[inode_id / iNODES_PER_BLOCK].inodeList[inode_id % iNODES_PER_BLOCK]), 0, sizeof(InodeDiskType));

	// Free i-node, if error return -2
	if (ifree(inode_id) < 0) {
		printf("Error! i-node of file %s couldn't be freed.\n", fileName);
		return -2;
	}

	//All the blocks referenced by the file are freed
	for(int i = 0;i < (inodosBlock[inode_id / iNODES_PER_BLOCK].inodeList[inode_id % iNODES_PER_BLOCK].numBlocks); i++){
		if (freeBlock(inodosBlock[inode_id / iNODES_PER_BLOCK].inodeList[inode_id % iNODES_PER_BLOCK].inodeTable[i]) < 0) {
		printf("Error! Block of file %s couldn't be freed.\n", fileName);
		return -2;
		}
	}

	//Update the file metadata
	//The first block of the device is used by superBlock, thus the offset applied to block id
	if(bwrite(DEVICE_IMAGE, (inode_id / iNODES_PER_BLOCK) + 1, (char *)&(inodosBlock[inode_id / iNODES_PER_BLOCK]))){
		printf("Error writing the file into disk\n");
		return -1;
	}

	printf("File %s removed.\n", fileName);
	return 0;
}

/*
 * @brief	Opens an existing file.
 * @return	The file descriptor if possible, -1 if file does not exist, -2 if the file is already opened
 	-3 if the file must be opened with integrity..
 */
int openFile(char *fileName)
{  
	//First of all we look if the file exist, if we didn't find we return -1
	int fileDescriptor = namei(fileName);

	//User may introduce a linkName so we search for the corresponding File
	if(inodosBlock[fileDescriptor / iNODES_PER_BLOCK].inodeList[fileDescriptor % iNODES_PER_BLOCK].type==T_LINK){
		fileDescriptor = inodosBlock[fileDescriptor / iNODES_PER_BLOCK].inodeList[fileDescriptor % iNODES_PER_BLOCK].pointsTo;
	}

	// If error return -1
	if (fileDescriptor < 0) {
		printf("Error! File %s doesn't exist.\n", fileName);
		return -1 ;
	}

	// If file is already opened return -2
	if (file_List[fileDescriptor].opened == 1) {
		printf("Error! File with name %s is already opened.\n", fileName);
		return -2;
	}
    
	// Error if file has integrity
	if (file_List[fileDescriptor].crc32_value != 0) {
		printf("Error! File with name %s cannot be opened without integrity.\n", fileName);
		return -3;
	}

	file_List[fileDescriptor].position = 0;	// Seek position = 0
	file_List[fileDescriptor].opened = 1;		// Open bit = 1
	file_List[fileDescriptor].currentBlock = 0;	// We are in data Block  0

	printf("File %s opened.\n", fileName);
	return fileDescriptor; 
}

/*
 * @brief	Closes a previously opened file.
 * @return	0 if success, -1 otherwise.
 */
int closeFile(int fileDescriptor)
{
	if (fileDescriptor < 0){
		printf("Error! Wrong file descriptor.\n");
		return -1;
	}

	//We look in the inode map if that file exists. 
	if (!sBlock.imap[fileDescriptor]){
		printf("Error! There is no file with such file descriptor.\n");
		return -1;
	}

	// If file wasn't opened return -1
	if (file_List[fileDescriptor].opened == 0) {
		printf("Error! File with file descriptor %d wasn't opened.\n", fileDescriptor);
		return -1;
	}

	// If file was opened with integrity return -1
	if (file_List[fileDescriptor].integrity == 1) {
		printf("Error! File with file descriptor %d was opened with integrity.\n", fileDescriptor);
		return -1;
	}

	file_List[fileDescriptor].position = 0;		// Seek position = 0
	file_List[fileDescriptor].opened = 0;		// Open bit = 0
	file_List[fileDescriptor].currentBlock = 0;	// We are in data Block  0

	printf("File with file descriptor %d closed.\n", fileDescriptor);
	return 0;
}


/*
 * @brief	Reads a number of bytes from a file and stores them in a buffer.
 * @return	Number of bytes properly read, -1 in case of error.
 */
int readFile(int fileDescriptor, void * buffer, int numBytes){

	// If error return -1
	if (numBytes <= 0 || file_List[fileDescriptor].opened == 0) {
		printf("Error! File with id %d couldn't be read.\n", fileDescriptor);
		return -1;
	}

	InodeDiskType * file = &(inodosBlock[fileDescriptor / iNODES_PER_BLOCK].inodeList[fileDescriptor % iNODES_PER_BLOCK]);
	int bytesRead = 0; //bytes currently read
	//numBytes = maximum bytes to read
	char * blockBuffer = calloc(BLOCK_SIZE, sizeof(char));
	
	if (numBytes > file->size){
		/*If the buffer is bigger than the file, the new 
		maximum number of bytes to write is file size*/
		numBytes = file->size;
	}
	//This variable store the number of blocks that will be read from the file. May not be whole blocks
	int blocksToRead;
	//Obtain the integer number of blocks to read. The result is ceiled.
	if(numBytes % BLOCK_SIZE){
		blocksToRead = (numBytes / BLOCK_SIZE) +1;
	}else {
		blocksToRead = numBytes / BLOCK_SIZE;
	}
	
	for (int i = 0; i < blocksToRead; i++){
		//The first three blocks of the disk are used for metadata, thus the offset
		if (bread(DEVICE_IMAGE, file->inodeTable[i]+3, blockBuffer) < 0) {
			printf("Error! Block %d of file with id %d couldn't be read.\n", file->inodeTable[i] + 3, fileDescriptor);
			return -1;
		}
		
		if (numBytes >= BLOCK_SIZE){//if the data block is completely written, an entire block is read
			memmove(buffer + bytesRead, blockBuffer, BLOCK_SIZE);
			numBytes -= BLOCK_SIZE;
			bytesRead += BLOCK_SIZE;
		}else { //The data block is not completely written (last block) read the remaining
			memmove(buffer + bytesRead, blockBuffer, file->size % BLOCK_SIZE);
			numBytes -= file->size % BLOCK_SIZE;
			bytesRead += file->size % BLOCK_SIZE;
		}
	}
	return bytesRead;
}

/*
 * @brief	Writes a number of bytes from a buffer and into a file.
 * @return	Number of bytes properly written, -1 in case of error.
 */
int writeFile(int fileDescriptor, void * buffer, int numBytes){

	// If error return -1
	if (numBytes <= 0) {
		printf("Error! File with id %d couldn't be written.\n", fileDescriptor);
		return -1;
	}

	InodeDiskType * file = &(inodosBlock[fileDescriptor / iNODES_PER_BLOCK].inodeList[fileDescriptor % iNODES_PER_BLOCK]);

	int bytesWritten = 0; //bytes currently writen
	//numBytes = maximum bytes to write
	char * blockBuffer = calloc(BLOCK_SIZE, sizeof(char));
	
	//Obtain the number last block of the file. If it is not completed, it will be used before allocating a new block
	int fileLastBlock = file->numBlocks - 1;
	
	//Position of the last byte of the file within its last block
	int lastByteInBlock = file->size % BLOCK_SIZE;
	
	int remainingFileSpace = MAX_FILE_SIZE - file->size;
	if (numBytes >= remainingFileSpace){
		/*If the buffer is bigger than the remaining free space, the new 
		maximum number of bytes to write is file size*/
		numBytes = remainingFileSpace;
	}

	//Read Take last block
	//The first three blocks are used for metadata, thus the offset used for block id
	bread(DEVICE_IMAGE, file->inodeTable[fileLastBlock] + 3, blockBuffer);

	//Compute the remaining free space in the last block 
	int currentBlockSpace = file->size % BLOCK_SIZE;
	//Write, if possible, some content of the buffer in the free space of last block
	if (numBytes > currentBlockSpace){
		memmove(blockBuffer + (lastByteInBlock), buffer, BLOCK_SIZE - lastByteInBlock);
		//The first three blocks are used for metadata, thus the offset used for block id
		bwrite(DEVICE_IMAGE, file->inodeTable[fileLastBlock] + 3, blockBuffer);
		numBytes -= (BLOCK_SIZE - lastByteInBlock);
		bytesWritten += (BLOCK_SIZE - lastByteInBlock);
		fileLastBlock++;
	} else {//If the current block is not filled completely, write has finished
		memmove(blockBuffer + (lastByteInBlock), buffer, numBytes);
		bwrite(DEVICE_IMAGE, file->inodeTable[fileLastBlock] + 3, blockBuffer);
		//Update file size before returning
		file->size += bytesWritten;
		return numBytes;
		
	}
	//while there are bytes to be written
	while(numBytes){
		//reset the buffer
		memset(blockBuffer, 0, BLOCK_SIZE);

		//Allocate new blocks until numBytes == 0
		/*There will never be allocated more blocks than possible (5 max) since 
		 at the begining of the function, if numBytes to write is bigger than the remaining free space,
		 numBytes is truncated, thus alloc will not be called if remaining file size is smaller than Block size
		*/
		int block_id = alloc();
		if (block_id < 0){
			return -1;		
		}
		file->inodeTable[fileLastBlock] = block_id;
		if (numBytes > BLOCK_SIZE){//The next block will be completely written
			memmove(blockBuffer, buffer + bytesWritten, BLOCK_SIZE);
			numBytes -= BLOCK_SIZE;
			bytesWritten += BLOCK_SIZE;
			fileLastBlock++;
		}else{//The next block will not be completely written
			memmove(blockBuffer, buffer + bytesWritten, numBytes);
			bytesWritten += numBytes;
			numBytes -= numBytes;
		}
		//The first three blocks are used for metadata, thus the offset used for block id
		bwrite(DEVICE_IMAGE, block_id + 3, blockBuffer);
	}
	//Update file size before returning
	file->size += bytesWritten;
	return bytesWritten;
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
	int inodeSize = inodosBlock[fileDescriptor / iNODES_PER_BLOCK].inodeList[fileDescriptor % iNODES_PER_BLOCK].size;
	if (whence == FS_SEEK_BEGIN) {
		file_List[fileDescriptor].position = 0;
		file_List[fileDescriptor].currentBlock = 0;
	}

	// Change file position to its current position plus an offset	
	else if (whence == FS_SEEK_CUR && file_List[fileDescriptor].position + offset <= inodeSize) {
		file_List[fileDescriptor].position += offset;
		file_List[fileDescriptor].currentBlock = file_List[fileDescriptor].position / BLOCK_SIZE;
	}

	// Change file position to the end
	else if (whence == FS_SEEK_END) {
		
		file_List[fileDescriptor].position = inodeSize;
		file_List[fileDescriptor].currentBlock = file_List[fileDescriptor].position / BLOCK_SIZE;
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
 * @return	0 if success, -1 if the file is corrupted, -2 if the file do not exist, 
 	-3 if the file could not be opened, -4 if the file has no integrity.
 */

int checkFile (char * fileName)
{	
	// locate the file
	int fileDescriptor = namei(fileName);

	//User may introduce a linkName so we search for the corresponding File
	if(inodosBlock[fileDescriptor / iNODES_PER_BLOCK].inodeList[fileDescriptor % iNODES_PER_BLOCK].type==T_LINK){
		fileDescriptor = inodosBlock[fileDescriptor / iNODES_PER_BLOCK].inodeList[fileDescriptor % iNODES_PER_BLOCK].pointsTo;
	}

	// Error if the file doesn't exist
	if (fileDescriptor == -1) {
		printf("Error! The file with name %s does not exist in the file system.\n", fileName);
		return -2;
	}
	// If error opening the file return -3
	else if (file_List[fileDescriptor].opened == 1) {
		printf("Error! The file %s is already opened without integrity.\n", fileName);
		return -3;
	}
	// If file without integrity return -4
	else if (file_List[fileDescriptor].crc32_value == 0) {
		printf("Error! The file with name %s doesn't have integrity value.\n", fileName);
		return -4;
	}

	//Open the file
	file_List[fileDescriptor].opened = 1;

	// Buffer to save file content and to compute CRC-32 value
	int inodeSize = inodosBlock[fileDescriptor / iNODES_PER_BLOCK].inodeList[fileDescriptor % iNODES_PER_BLOCK].size;
	char * buffer = calloc(inodeSize, sizeof(char));

	// Read file
	readFile(fileDescriptor, buffer, inodeSize);

	// Compute CRC-32 value
	uint32_t val = CRC32((const unsigned char*)buffer, inodeSize);

	// Check if corrupted file
	if (val != file_List[fileDescriptor].crc32_value) {
		printf("Warning! The file with name %s is corrupted.\n", fileName);
		closeFile(fileDescriptor);
		return -1;
	}

	// Close file
	closeFile(fileDescriptor);

    return 0;
}

/*
 * @brief	Include integrity on a file.
 * @return	0 if success, -1 if the file does not exists, -2 in case of error.
 */

int includeIntegrity (char * fileName)
{	
	// Open file
	int fileDescriptor = openFile(fileName);

	// Error if the file doesn't exist
	if (fileDescriptor == -1) {
		printf("Error! The file with name %s does not exist in the file system.\n", fileName);
		closeFile(fileDescriptor);
		return -1;
	}

	// If error opening the file return -2
	else if (fileDescriptor == -2) {
		printf("Error! The file with name %s couldn't be opened.\n", fileName);
		closeFile(fileDescriptor);
		return -2;
	}

	// Error if file has already integrity
	if (file_List[fileDescriptor].crc32_value != 0) {
		printf("Error! The file with name %s has already integrity.\n", fileName);
		closeFile(fileDescriptor);
		return -2;
	}

	// Buffer to save file content and to compute CRC-32 value
	int inodeSize = inodosBlock[fileDescriptor / iNODES_PER_BLOCK].inodeList[fileDescriptor % iNODES_PER_BLOCK].size;
	char * buffer = calloc(inodeSize, sizeof(char));

	// Read file
	readFile(fileDescriptor, buffer, inodeSize);

	// Add integrity
	file_List[fileDescriptor].crc32_value = CRC32((const unsigned char*)buffer, inodeSize);

	// Close file
	closeFile(fileDescriptor);

    return 0;
}

/*
 * @brief	Opens an existing file and checks its integrity
 * @return	The file descriptor if possible, -1 if file does not exist, -2 if the file is corrupted, -3 in case of error
 */
int openFileIntegrity(char *fileName)
{
	// First the integrity of the file is checked

	//The file do not exist
	int ret = checkFile(fileName);
	if (ret == -2) {
		return -1;
	}
	//Corrupted file
	else if (ret == -1) {
		return -2;
	}
	//The file could not be opened with integrity
	else if (ret == -3 || ret == -4) {
		return -3;
	}

	//All the checks are OK. Locate the file
	int fileDescriptor = namei(fileName);

	//User may introduce a linkName so we search for the corresponding File
	if(inodosBlock[fileDescriptor / iNODES_PER_BLOCK].inodeList[fileDescriptor % iNODES_PER_BLOCK].type==T_LINK){
		fileDescriptor = inodosBlock[fileDescriptor / iNODES_PER_BLOCK].inodeList[fileDescriptor % iNODES_PER_BLOCK].pointsTo;
	}

	// Open file
	file_List[fileDescriptor].integrity = 1;	// Opened with integrity bit = 1
	file_List[fileDescriptor].position = 0;		// Seek pointer = 0
	file_List[fileDescriptor].opened = 1;		// Open bit = 1
	file_List[fileDescriptor].currentBlock = 0;	// We are in data Block  0

	printf("File %s opened with integrity.\n", fileName);
	return fileDescriptor;
}

/*
 * @brief	Closes a file and updates its integrity.
 * @return	0 if success, -1 otherwise.
 */
int closeFileIntegrity(int fileDescriptor)
{
    if (fileDescriptor < 0){
		printf("Error! Wrong file descriptor.\n");
		return -1;
	}

	// We look in the inode map if that file exists. 
	if (!sBlock.imap[fileDescriptor]){
		printf("Error! There is no file with such file descriptor.\n");
		return -1;
	}

	// If file wasn't opened return -1
	if (file_List[fileDescriptor].opened == 0) {
		printf("Error! File with file descriptor %d wasn't opened.\n", fileDescriptor);
		return -1;
	}

	// If file wasn't opened with integrity check return -1
	if (file_List[fileDescriptor].integrity == 0) {
		printf("Error! File with file descriptor %d wasn't opened with integrity.\n", fileDescriptor);
		return -1;
	}

	// Buffer to save file content and to compute CRC-32 value
	int inodeSize = inodosBlock[fileDescriptor / iNODES_PER_BLOCK].inodeList[fileDescriptor % iNODES_PER_BLOCK].size;
	char * buffer = calloc(inodeSize, sizeof(char));

	// Read file
	readFile(fileDescriptor, buffer, inodeSize);

	// Add integrity
	file_List[fileDescriptor].crc32_value = CRC32((const unsigned char*)buffer, inodeSize);

	// Close file
	file_List[fileDescriptor].position = 0;		// Seek position = 0
	file_List[fileDescriptor].opened = 0;		// Open bit = 0
	file_List[fileDescriptor].currentBlock = 0;	// We are in data Block  0
	file_List[fileDescriptor].integrity = 0;	// Opened with integrity bit = 0

	printf("File with file descriptor %d closed.\n", fileDescriptor);
	return 0;
}

/*
 * @brief	Creates a symbolic link to an existing file in the file system.
 * @return	0 if success, -1 if file does not exist, -2 in case of error.
 */
int createLn(char *fileName, char *linkName)
{
   // Error if the link already exists
	if (namei(linkName) != -1) {
		printf("The link with name %s already exists in the file system.\n", linkName);
		return -2;
	}

	// Block and i-node IDs
	int inode_id;

	// Allocate a new i-node
	inode_id = ialloc();

	// If error return -1
	if (inode_id < 0) {
		printf("Error! i-node of link %s couldn't be allocated.\n", linkName);
		return -2;
	}

	//Look for inode of the file
	int inodeFile = namei(fileName);
	if (inodeFile < 0) {
		printf("The File with name %s doesn't exist in the file system.\n", fileName);
		return -1;
	}
	
	//Store the id of the inode
	inodosBlock[inode_id / iNODES_PER_BLOCK].inodeList[inode_id % iNODES_PER_BLOCK].pointsTo = inodeFile;
	inodosBlock[inode_id / iNODES_PER_BLOCK].inodeList[inode_id % iNODES_PER_BLOCK].type = T_LINK;
	strcpy(inodosBlock[inode_id / iNODES_PER_BLOCK].inodeList[inode_id % iNODES_PER_BLOCK].name, linkName);

	printf("Link %s created.\n", linkName);
	return 0; 
}

/*
 * @brief 	Deletes an existing symbolic link
 * @return 	0 if the file is correct, -1 if the symbolic link does not exist, -2 in case of error.
 */
int removeLn(char *linkName)
{
    int inode_id = namei(linkName);

	// Error if the file doesn't exist
	if (inode_id < 0) {
		printf("The link %s does not exists.\n", linkName);
		return -1;
	}

	if(inodosBlock[inode_id / iNODES_PER_BLOCK].inodeList[inode_id % iNODES_PER_BLOCK].type==T_FILE){
		printf("Error! %s is a file, not a link\n", linkName);
		return -2;
	}
	memset(&(inodosBlock[inode_id / iNODES_PER_BLOCK].inodeList[inode_id % iNODES_PER_BLOCK]), 0, sizeof(InodeDiskType));

	// Free i-node, if error return -2
	if (ifree(inode_id) < 0) {
		printf("Error! i-node of link %s couldn't be freed.\n", linkName);
		return -2;
	}

	printf("Link %s removed.\n", linkName);
	return 0;
}
