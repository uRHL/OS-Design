
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
 * @brief 	Generates the proper file system structure in a storage device, as designed by the student.
 * @return 	0 if success, -1 otherwise.
 */
int mkFS(long deviceSize)
{
	if(deviceSize < MIN_DISK_SIZE || deviceSize > MAX_DISK_SIZE){
		printf("Disk size should be between 460KB and 600KB\n");
		return -1;
	}
	 // setup with default values the superblock, maps, and i-nodes	 
	    sBlock.magicNumber = 0x000D5500;   
		sBlock.numInodes = MAX_iNODE_NUM;

		//Not needed. See metadata.h
	    //sBlock.numBlocksInodeMap = ; //Cuantos bloques necesito para mapear todos los nodos (todos los punteros)
	    //sBlock.numBlocksBlockMap = 48; //Cuantos bloques necesito para mapear todos los data blocks (todos los punteros)

	    sBlock.firstInode = 0; //Superblock
		
		//49 = num Inode blocks + num superblock
	    sBlock.numDataBlocks = (deviceSize / BLOCK_SIZE) - (49);
	    sBlock.deviceSize = deviceSize;


	    for (int i=0; i<sBlock.numInodes; i++){           
	    	sBlock.imap[i] = 0; // free
	    	}
		/*Since bmap contains positions for the maximum possible number of datablocks, 
		some of them may actually refer to an unexistent block. 
		Those positions, (when i is greater than numDataBlocks) should be marked as invalid*/
	    for (int i =0; i<MAX_NUM_DATABLOCKS; i++){
			if( i>= sBlock.numDataBlocks){
				sBlock.bmap[i] = -1; // invalid 
			}else{
				sBlock.bmap[i] = 0; // free 
			}	    	
	    }
	    for (int i=0; i<sBlock.numInodes; i++){
	    	memset(&(inodos[i]), 0, sizeof(InodeDiskType) );
	    }          
	return 0;
}

/*
 * @brief 	Mounts a file system in the simulated device.
 * @return 	0 if success, -1 otherwise.
 */
int mountFS(void)
{
	// To write block 0 from sBlock into disk    
	bwrite(DEVICE_IMAGE, 0, (char *)&(sBlock) );

	/* Not needed. Imap and bmap are now contained in the supeerblock 
	    
	// To write the i-node map to disk    
	for (int i=0; i<sBlock.numBlocksInodeMap; i++){
		bwrite(DEVICE_IMAGE, 1+i, ((char *)sBlock.imap + i*BLOCK_SIZE)) ;    
		}            
	// To write the block map to disk    
	for (int i =0; i<sBlock.numBlocksBlockMap; i++){
		bwrite(DEVICE_IMAGE, 1+i+sBlock.numBlocksInodeMap, ((char *)sBlock.bmap + i*BLOCK_SIZE));
	} 
	*/          
	// To write the i-nodes to disk
	for (int i=0; i<(sBlock.numInodes*sizeof(InodeDiskType)/BLOCK_SIZE); i++){
		bwrite(DEVICE_IMAGE, i+sBlock.firstInode, ((char *)inodos + i*BLOCK_SIZE));
	} 
    return 1;
}

/*
 * @brief 	Unmounts the file system from the simulated device.
 * @return 	0 if success, -1 otherwise.
 */
int unmountFS(void)
{
	return -1;
}

/*
 * @brief	Creates a new file, provided it it doesn't exist in the file system.
 * @return	0 if success, -1 if the file already exists, -2 in case of error.
 */
int createFile(char *fileName)
{
	return -2;
}

/*
 * @brief	Deletes a file, provided it exists in the file system.
 * @return	0 if success, -1 if the file does not exist, -2 in case of error..
 */
int removeFile(char *fileName)
{
	return -2;
}

/*
 * @brief	Opens an existing file.
 * @return	The file descriptor if possible, -1 if file does not exist, -2 in case of error..
 */
int openFile(char *fileName)
{
	return -2;
}

/*
 * @brief	Closes a file.
 * @return	0 if success, -1 otherwise.
 */
int closeFile(int fileDescriptor)
{
	return -1;
}

/*
 * @brief	Reads a number of bytes from a file and stores them in a buffer.
 * @return	Number of bytes properly read, -1 in case of error.
 */
int readFile(int fileDescriptor, void *buffer, int numBytes)
{
	return -1;
}

/*
 * @brief	Writes a number of bytes from a buffer and into a file.
 * @return	Number of bytes properly written, -1 in case of error.
 */
int writeFile(int fileDescriptor, void *buffer, int numBytes)
{
	return -1;
}

/*
 * @brief	Modifies the position of the seek pointer of a file.
 * @return	0 if succes, -1 otherwise.
 */
int lseekFile(int fileDescriptor, long offset, int whence)
{
	return -1;
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
