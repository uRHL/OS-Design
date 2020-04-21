
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
	    sBlock.numDataBlocks = MAX_NUM_DATABLOCKS;

		//Not needed. See metadata.h
	    //sBlock.numBlocksInodeMap = ; //Cuantos bloques necesito para mapear todos los nodos (todos los punteros)
	    //sBlock.numBlocksBlockMap = 48; //Cuantos bloques necesito para mapear todos los data blocks (todos los punteros)

	    sBlock.firstInode = 1; //Superblock ocupies 1
		
		//49 = num Inode blocks + num superblock
		sBlock.firstDataBlock = 49; //superblock in 0 plus 48 inodes
	    sBlock.deviceSize = deviceSize;

	    for (int i=0; i<sBlock.numInodes; i++){           
	    	sBlock.imap[i] = 0; // free
	    	}
	    for (int i =0; i<MAX_NUM_DATABLOCKS; i++){
			sBlock.datablockmap[i] = 0; // free 
	    }
	    for (int i=0; i<sBlock.numInodes; i++){
	    	memset(&(inodos[i]), 0, sizeof(InodeDiskType) );
	    } 
		syncronizeWithDisk();

		/*//we also prepare the file array
		for(int i=0;i<sBlock.numInodes;i++){
			file_List[i].position=0;
			file_List[i].opened=0;
		}
*/
	return 0;
}

/*
 * @brief 	Syncronizes local filesytem with disk
 * @return 	0 if success, -1 otherwise.
 */
int syncronizeWithDisk()
{
	// To write block 0 from sBlock into disk    
	bwrite(DEVICE_IMAGE, 0, (char *)&(sBlock) ); 
	// To write the i-nodes to disk    
	for (int i=0; i<(sBlock.numInodes*sizeof(InodeDiskType)/BLOCK_SIZE); i++){
		bwrite(DEVICE_IMAGE, 1+i, ((char *)inodos + i*BLOCK_SIZE)); //WE start writing in block 1 because all the previous information is stored in the superblock
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
	bread(DEVICE_IMAGE, 0, (char *)&(sBlock) );           
	// To write the i-nodes to disk
	for (int i=0; i<(sBlock.numInodes*sizeof(InodeDiskType)/BLOCK_SIZE); i++){
		bread(DEVICE_IMAGE, i+1, ((char *)inodos + i*BLOCK_SIZE));
	} 
    return 1;
}

/*
 * @brief 	Unmounts the file system from the simulated device.
 * @return 	0 if success, -1 otherwise.
 */
int unmountFS(void)
{
	for(int i=0;i<MAX_iNODE_NUM;i++){
		if (file_List[i].opened==1){
			printf("There are files that haven't been closed\n");
			return -1;
		}
	}
	syncronizeWithDisk();
	return 0;
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
	int inode_id=-1;   
	//First of all we look if the file exist 
	for (int i=0; i<sBlock.numInodes; i++){
		if (! strcmp(inodos[i].name, fileName)){ //This means strcmp returned 0 so they are equal
			inode_id= i;
		}
	}    
	//If we didn't find we return -1
	if (inode_id < 0){
		return inode_id ;
	}           

	file_List[inode_id].position = 0;    
	file_List[inode_id].opened   = 1;    
	return inode_id; 
}

/*
 * @brief	Closes a file.
 * @return	0 if success, -1 otherwise.
 */
int closeFile(int fileDescriptor)
{

	if (fileDescriptor < 0){
		return -1;
	}
	//We look in the inode map if that file has been created. 
	if (!bitmap_getbit(sBlock.imap, fileDescriptor)){
		printf("There is no file with that id\n");
		return -1;
	}
	file_List[fileDescriptor].position=0;
	file_List[fileDescriptor].opened=0;
	return 0;
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
