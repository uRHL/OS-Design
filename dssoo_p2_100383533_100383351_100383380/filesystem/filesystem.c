
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
 * @brief 	Allocate blocks when writing more than 1
 * @return 	block id if success, -1 otherwise.
 */
int allocateInWrite(int fileDescriptor){	
	int actualBlock = file_List[fileDescriptor].currentBlock;
	int newBlock_id = 0;
	if (actualBlock < 4){ //If it's 4 we cant add more blocks
		newBlock_id = alloc();
		if (newBlock_id < 0){
			return -1; //Problem allocating the block
		}
		actualBlock++;
		
		/*
		inodos[fileDescriptor].inodeTable[actualBlock]=newBlock_id;
		inodos[fileDescriptor].numBlocks++;
		*/
		inodosBlock[fileDescriptor/iNODES_PER_BLOCK].inodeList[fileDescriptor%iNODES_PER_BLOCK].inodeTable[actualBlock] = newBlock_id;
		inodosBlock[fileDescriptor/iNODES_PER_BLOCK].inodeList[fileDescriptor%iNODES_PER_BLOCK].numBlocks++;
		file_List[fileDescriptor].currentBlock = actualBlock;
	}
	return newBlock_id;
}

/*
 * @brief 	Search block with position equal to offset of i-node inodo_id
 * @return 	block id if success, -1 otherwise.
 */
/*int bmap(int inodo_id, int offset)
{
	int b[BLOCK_SIZE / 4];

	// If offset is lower than the block size, return the direct block
	if (offset < BLOCK_SIZE) {
		return inodos[inodo_id].inodeTable[0];
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
}*/

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
	if (syncronizeWithDisk() < 0) {
		printf("Error! Couldn't synchronize with disk.\n");
		return -1;
	}

	printf("New file system made.\n");
	return 0;
}

/*
 * @brief 	Syncronizes local filesytem with disk
 * @return 	0 if success, -1 otherwise.
 */
int syncronizeWithDisk()
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
	int fileDescriptor = namei(fileName);

	//User may introduce a linkName so we search for the corresponding File
	if(inodosBlock[fileDescriptor / iNODES_PER_BLOCK].inodeList[fileDescriptor % iNODES_PER_BLOCK].type==T_LINK){
		fileDescriptor=inodosBlock[fileDescriptor / iNODES_PER_BLOCK].inodeList[fileDescriptor % iNODES_PER_BLOCK].pointsTo;
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
		return -2;
	}

	file_List[fileDescriptor].position = 0;	// Seek position = 0
	file_List[fileDescriptor].opened = 1;		// Open bit = 1
	file_List[fileDescriptor].currentBlock = 0;	// We are in data Block  0

	printf("File %s opened.\n", fileName);
	return fileDescriptor; 
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

	// If file wasn't opened return -1
	if (file_List[fileDescriptor].opened == 0) {
		printf("Error! File with file descriptor %d wasn't opened.\n", fileDescriptor);
		return -1;
	}

	// If file was opened with integrity return -1
	if (file_List[fileDescriptor].integrity == 1) {
		printf("Error! File with file descriptor %d wasn't opened without integrity.\n", fileDescriptor);
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
int readFile(int fileDescriptor, void *buffer, int numBytes)
{
	char b[BLOCK_SIZE];
	int block_id;

	// If error return -1
	if (numBytes <= 0) {
		printf("Error! File with id %d couldn't be read.\n", fileDescriptor);
		return -1;
	}

	// If file position plus number of bytes to be read is greater that the file size,
	// the number of bytes to be read is resized to what's left of the file

	int inodeSize = inodosBlock[fileDescriptor / iNODES_PER_BLOCK].inodeList[fileDescriptor % iNODES_PER_BLOCK].size;
	if (file_List[fileDescriptor].position + numBytes > inodeSize) {
		numBytes = inodeSize - file_List[fileDescriptor].position;
	}

	int bytesRead = numBytes; //auxiliary variable used fordward in the function

	//Part 1, finish reading the current block
	int remainingBlockFreeSpace =file_List[fileDescriptor].position % BLOCK_SIZE;
	block_id = file_List[fileDescriptor].currentBlock;

	if (block_id < 0) {
		printf("Error! Block of file with id %d couldn't be read.\n", fileDescriptor);
		return -1;
	}

	if (bread(DEVICE_IMAGE, block_id, b) < 0) {
		printf("Error! Block %d of file with id %d couldn't be read.\n", block_id, fileDescriptor);
		return -1;
	}
	// Save content to buffer
	memmove(buffer, b + (file_List[fileDescriptor].position % BLOCK_SIZE), remainingBlockFreeSpace); //Así cogemos la posición en ese bloque
	// Increase file position
	file_List[fileDescriptor].position += remainingBlockFreeSpace;
	int actualBlock= file_List[fileDescriptor].currentBlock;
	file_List[fileDescriptor].currentBlock= inodosBlock[fileDescriptor / iNODES_PER_BLOCK].inodeList[fileDescriptor % iNODES_PER_BLOCK].inodeTable[actualBlock + 1];//Comprobar que esto funciones realmente así
	numBytes -= remainingBlockFreeSpace;
	//End part 1

	//Part 2: iterate through whole disk blocks
	int VueltasAlLoop = numBytes / BLOCK_SIZE; //By truncating numBytes, the number of whole blocks is obtained
	for(int i = 0; i < VueltasAlLoop; i++){
		block_id = file_List[fileDescriptor].currentBlock;

		if (block_id < 0) {
			printf("Error! Block of file with id %d couldn't be read.\n", fileDescriptor);
			return -1;
		}
		//Comprobar si b se trunca y podemos hacerlo directamente asi o hay que reiniciarlo
		if (bread(DEVICE_IMAGE, block_id, b) < 0) {
			printf("Error! Block %d of file with id %d couldn't be read.\n", block_id, fileDescriptor);
			return -1;
		}
		// Save content to buffer
		memmove(buffer+remainingBlockFreeSpace+i*BLOCK_SIZE, b + (file_List[fileDescriptor].position % BLOCK_SIZE), BLOCK_SIZE); //Esta vez la posición debería ser 0 durante el calculo, porque empezamos bloque
		// Increase file position
		file_List[fileDescriptor].position += BLOCK_SIZE;
		actualBlock= file_List[fileDescriptor].currentBlock;

		file_List[fileDescriptor].currentBlock = inodosBlock[fileDescriptor / iNODES_PER_BLOCK].inodeList[fileDescriptor % iNODES_PER_BLOCK].inodeTable[actualBlock+1];//Comprobar que esto funciones realmente así
		numBytes -= BLOCK_SIZE;
	}
	//End part 2

	//Parte 3: Leemos lo que quede
	if(numBytes > 0){
		block_id = file_List[fileDescriptor].currentBlock;

		if (block_id < 0) {
			printf("Error! Block of file with id %d couldn't be read.\n", fileDescriptor);
			return -1;
		}

		if (bread(DEVICE_IMAGE, block_id, b) < 0) {
			printf("Error! Block %d of file with id %d couldn't be read.\n", block_id, fileDescriptor);
			return -1;
		}
		// Save content to buffer
		memmove(buffer + remainingBlockFreeSpace + VueltasAlLoop * BLOCK_SIZE, b + (file_List[fileDescriptor].position % BLOCK_SIZE), numBytes);
		// Increase file position
		file_List[fileDescriptor].position += numBytes;
		//En esta version no superamos el bloque actual por lo que no lo actualizamos
	}
	//Fin Parte 3
	return bytesRead;
}

/*
 * @brief	Writes a number of bytes from a buffer and into a file.
 * @return	Number of bytes properly written, -1 in case of error.
 */
int writeFile(int fileDescriptor, void *buffer, int numBytes)
{
	char b[BLOCK_SIZE];
	int block_id;

	// If error return -1
	if (numBytes <= 0) {
		printf("Error! File with id %d couldn't be written.\n", fileDescriptor);
		return -1;
	}
	// If file position plus number of bytes to be written is greater that the file size,
	// the number of bytes to be written is resized to what's left of the file
	if (file_List[fileDescriptor].position + numBytes > 10240) {
		numBytes = 10240 - file_List[fileDescriptor].position;
	}
	int bytesWritten = numBytes;


    //Parte 1
	int remainingBlockFreeSpace =file_List[fileDescriptor].position % BLOCK_SIZE;
	block_id = file_List[fileDescriptor].currentBlock;
	
	if (bread(DEVICE_IMAGE, block_id, b) < 0) {
		printf("Error! Block %d of file with id %d couldn't be read.\n", block_id, fileDescriptor);
		return -1;
	}
	// Write content to file
	memmove(b + file_List[fileDescriptor].position%BLOCK_SIZE, buffer, remainingBlockFreeSpace);
	bwrite(DEVICE_IMAGE, block_id, b);
	// Increase file position
	file_List[fileDescriptor].position += remainingBlockFreeSpace;
	// Increase file size
	inodosBlock[fileDescriptor / iNODES_PER_BLOCK].inodeList[fileDescriptor % iNODES_PER_BLOCK].size += remainingBlockFreeSpace;
	numBytes -=remainingBlockFreeSpace;

	//Now we need to allocate a new data block
	int newBlock_id = allocateInWrite(fileDescriptor);
	if(newBlock_id<0){
		return -1;
	}
	//End part 1

	//Loop part 2
	int VueltasAlLoop = numBytes/BLOCK_SIZE; //Al truncar, obtendremos los bloques enteros que leemos
	for(int i=0; i<VueltasAlLoop;i++){
		if (bread(DEVICE_IMAGE, newBlock_id, b) < 0) {
			printf("Error! Block %d of file with id %d couldn't be read.\n", block_id, fileDescriptor);
			return -1;
		}
	// Write content to file
	memmove(b + file_List[fileDescriptor].position%BLOCK_SIZE, buffer+remainingBlockFreeSpace+i*BLOCK_SIZE, BLOCK_SIZE);
	bwrite(DEVICE_IMAGE, block_id, b);
	// Increase file position
	file_List[fileDescriptor].position += BLOCK_SIZE;
	// Increase file size
	
	inodosBlock[fileDescriptor / iNODES_PER_BLOCK].inodeList[fileDescriptor % iNODES_PER_BLOCK].size += BLOCK_SIZE;
	numBytes -=BLOCK_SIZE;
	newBlock_id=allocateInWrite(fileDescriptor);
		if(newBlock_id<0){
			return -1;
		}
	}
	//End part 2

	//Part 3:The rest
	if(numBytes>0){
		if (bread(DEVICE_IMAGE, block_id, b) < 0) {
			printf("Error! Block %d of file with id %d couldn't be read.\n", block_id, fileDescriptor);
			return -1;
		}	
	memmove(b + file_List[fileDescriptor].position%BLOCK_SIZE, buffer+remainingBlockFreeSpace+VueltasAlLoop*BLOCK_SIZE, numBytes);
	bwrite(DEVICE_IMAGE, block_id, b);	
	file_List[fileDescriptor].position += numBytes;
	// Increase file size
	
	inodosBlock[fileDescriptor / iNODES_PER_BLOCK].inodeList[fileDescriptor % iNODES_PER_BLOCK].size += numBytes;
	}
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
		file_List[fileDescriptor].currentBlock = file_List[fileDescriptor].position % BLOCK_SIZE;
	}

	// Change file position to the end
	else if (whence == FS_SEEK_END) {
		
		file_List[fileDescriptor].position = inodeSize;
		file_List[fileDescriptor].currentBlock = file_List[fileDescriptor].position % BLOCK_SIZE;
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
	// Open file
	int fileDescriptor = openFile(fileName);

	// Error if the file doesn't exist
	if (fileDescriptor == -1) {
		printf("Error! The file with name %s does not exist in the file system.\n", fileName);
		closeFile(fileDescriptor);
		return -2;
	}

	// If error opening the file return -3
	else if (fileDescriptor == -2) {
		printf("Error! The file with name %s couldn't be opened.\n", fileName);
		closeFile(fileDescriptor);
		return -3;
	}

	// If file without integrity return -4
	else if (file_List[fileDescriptor].crc32_value == 0) {
		printf("Error! The file with name %s doesn't have integrity value.\n", fileName);
		closeFile(fileDescriptor);
		return -4;
	}

	// Buffer to save file content and to compute CRC-32 value
	int inodeSize = inodosBlock[fileDescriptor / iNODES_PER_BLOCK].inodeList[fileDescriptor % iNODES_PER_BLOCK].size;
	char buffer[inodeSize];

	// Read file
	readFile(fileDescriptor, &buffer, strlen(buffer));

	// Compute CRC-32 value
	uint32_t val = CRC32((const unsigned char*)&(buffer), strlen(buffer));

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
	char buffer[inodeSize];

	// Read file
	readFile(fileDescriptor, &buffer, strlen(buffer));

	// Add integrity
	file_List[fileDescriptor].crc32_value = CRC32((const unsigned char*)&buffer, strlen(buffer));

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
	// Check file integrity
	if (checkFile(fileName) == -2) {
		return -1;
	}

	else if (checkFile(fileName) == -1) {
		return -2;
	}

	else if (checkFile(fileName) == -3) {
		return -3;
	}

	else if (checkFile(fileName) == -4) {
		return -3;
	}

	// Open file
	int fileDescriptor = openFile(fileName);

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

	// We look in the inode map if that file has been created. 
	if (!bitmap_getbit(sBlock.imap, fileDescriptor)){
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
	char buffer[inodeSize];

	// Read file
	readFile(fileDescriptor, &buffer, strlen(buffer));

	// Add integrity
	file_List[fileDescriptor].crc32_value = CRC32((const unsigned char*)&buffer, strlen(buffer));

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
	inodosBlock[inode_id / iNODES_PER_BLOCK].inodeList[inode_id % iNODES_PER_BLOCK].pointsTo= inodeFile;
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
