
/*
 *
 * Operating System Design / Diseño de Sistemas Operativos
 * (c) ARCOS.INF.UC3M.ES
 *
 * @file 	test.c
 * @brief 	Implementation of the client test routines.
 * @date	07/05/2020
 *
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "filesystem/filesystem.h"
#include "filesystem/auxiliary.h"


// Color definitions for asserts
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_BLUE "\x1b[34m"

#define N_BLOCKS 250					  // Number of blocks in the device
#define DEV_SIZE N_BLOCKS * BLOCK_SIZE // Device size, in bytes

int main()
{
	//Auxiliary variables for the different tests
	int ret1, ret2, fd1, fd2;
	char * buffer = calloc(MAX_FILE_SIZE, sizeof(char));
	char * oversizedBuffer = calloc(MAX_FILE_SIZE + 1, sizeof(char));
	///////

	/*
		Test ID: 1 
		Objective: Verify F1.2
		Procedure: Mount a non existent FS
		Expected output: -1
	*/
	ret1 = mountFS();
	if(ret1 == -1){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 1 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 1 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
		
	/*
		Test ID: 2
		Objective: Verify F1.1, F8, NTF1, NTF2, NTF3, NTF4, NTF6
		Procedure: Make a FS with a valid device size
		Expected output: 0
	*/
	ret1 = mkFS(DEV_SIZE); //DEV_SIZE = 250 * BLOCK_SIZE
	if(ret1 == 0){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 2 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 2 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}

	/*
		Test ID: 3
		Objective: Verify F1.1, F8, NTF1, NTF2, NTF3, NTF4, NTF6
		Procedure: make a FS with invalid device size
		Expected output: -1
	*/
	ret1 = mkFS(0 * DEV_SIZE); //DEV_SIZE = 250 * BLOCK_SIZE
	if(ret1 == -1){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 3 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 3 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}

	/*
		Test ID: 4
		Objective: Verify F1.2
		Procedure: mount the created file system 
		Expected output: 0
	*/
	ret1 = mountFS();
	if(ret1 == 0){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 4 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 4 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}

	/*
		Test ID: 5 
		Objective: Verify F1.4 
		Procedure: Create a file
		Expected output: 0
	*/
	ret1 = createFile("/test.txt"); 
	if(ret1 == 0){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 5 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 5 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}

	/*
		Test ID: 6
		Objective: Verify F1.4
		Procedure: create two files with the same name 
		Expected output: -1
	*/
	ret1 = createFile("/test.txt");
	if(ret1 == -1){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 6 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 6 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}

	/*
		Test ID: 7 
		Objective: Verify F1.4, NTF1
		Procedure: Create the 49th file
		Expected output: 0 (1st) 0 (2nd) 0 (3rd) ... -2 (48th)
	*/
	char name[MAX_NAME_LENGTH];
	for (int i = 2; i < MAX_iNODE_NUM + 2; i++){
		sprintf(name, "/test%d.txt", i);
		ret1 = createFile(name);
		//for the first 47 files there should be no error (ret1 == 0) since there are inodes avilable
		if ((i <= MAX_iNODE_NUM)){
			if(ret1 == -1){
				fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 7A ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
				return -1;
			}			
		} else {
			if (ret1 == -2){ //the 49th file should not be created
				fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 7 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
			} else{
				fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 7B ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
				return -1;
			}
		}
		
		
	}

	/*
		Test ID: 8 
		Objective: Verify F1.5
		Procedure: remove a file
		Expected output: 0
	*/
	ret1 = removeFile("/test.txt");
	if(ret1 == 0){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 8 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 8 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}

	/*
		Test ID: 9 
		Objective: Verify F1.5
		Procedure: remove a file that do not exists
		Expected output: -1
	*/
	ret1 = removeFile("empty");
	if(ret1 == -1){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 9 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 9 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}

	/*
		Test ID: 10
		Objective: Verify F1.5, F1.15 
		Procedure: Create a link and try to remove it using removeFile() 
		Expected output: 0 -2
	*/
	ret1 = createLn("/test2.txt", "/testLn");
	ret2 = removeFile("/testLn");
	if(ret1 == 0 && ret2 == -2){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 10 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 10 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}

	/*
		Test ID: 11 
		Objective: Verify F1.6, NTF12
		Procedure: Close with integrity a file that was opened without it 
		Expected output: 1 -1
	*/
	fd1 = namei("/test2.txt");
	ret1 = closeFileIntegrity(fd1);
	//The inode num is used as fd. Therefore for this file fd = 1. 
	//For every case, it must hold that 0 <= fd < 48 (MAX_iNODE_NUM)
	if(fd1 == 1 && ret1 == -1){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 11 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 11 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}

	/*
		Test ID: 12
		Objective: Verify F1.11, F5, F8
		Procedure: Try to open with a integrity a file that does not have it 
		Expected output: -3
	*/
	ret1 = openFileIntegrity("/test3.txt");
	if(ret1 == -3){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 12 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 12 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}

	/*
		Test ID: 13
		Objective: Verify F1.6, F1.7
		Procedure: Close the previously opened file and try to open it using the link 
		Expected output: 0 1
	*/
	//The fd of the opened file was stored in fd1 during test 11
	ret1 = closeFile(fd1);
	fd1 = openFile("/testLn");
	if(ret1 == 0 && fd1 == 1){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 13 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 13 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}

/*
		Test ID: 14
		Objective: Verify F1.6 
		Procedure: Open a non existen file 
		Expected output: -1
	*/
	ret1 = openFile("empty");
	if(ret1 == -1){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 14 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 14 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}

/*
		Test ID: 15
		Objective: Verify F1.7 
		Procedure: Close a non existent file
		Expected output: -1
	*/
	ret1 = closeFile(-1);
	if(ret1 == -1){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 15 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 15 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}

/*
		Test ID: 16
		Objective: Verify F1.8
		Procedure: read from a new file (opened). 0 bytes should be read since the file has no written content yet. 
		Expected output: 0 (bytes read)
	*/
	//The file "/test2.txt" was previously opened during test 13
	ret1 = readFile(fd1, buffer, 25);
	if(ret1 == 0){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 16 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 16 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}

/*
		Test ID: 17
		Objective: Verify F1.8
		Procedure: read from a closed file 
		Expected output: -1
	*/
	//File 3 is not opened
	fd2 = namei("/test3.txt");
	closeFile(fd2);
	ret1 = readFile(fd2, buffer, 25);
	if(ret1 == -1){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 17 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		printf("ret1 = %d\n", ret1);
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 17 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	//buffer no longer used
	//free(buffer);

/*
		Test ID: 18
		Objective: Verify F1.9, F7 
		Procedure: write a file up to the maximum file size, using an oversized buffer
		Expected output: 10240 (bytes written)
	*/
	//Filling the buffer with some content
	for (char * pt = oversizedBuffer; pt < oversizedBuffer + MAX_FILE_SIZE + 1; pt++){
		*pt = 'f';
	}
	//The file "/test2.txt" (fd1) was previously opened during test 13
	ret1 = writeFile(fd1, oversizedBuffer, MAX_FILE_SIZE + 1);

/*
	//DEBUG
	//Filling the buffer with some content
	for (char * pt = oversizedBuffer; pt < oversizedBuffer + MAX_FILE_SIZE + 1; pt++){
		*pt = '0';
	}
	readFile(fd1, oversizedBuffer, MAX_FILE_SIZE);
	//DEBUG
	*/
	if(ret1 == MAX_FILE_SIZE){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 18 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 18 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	/*
		Test ID: 19
		Objective: Verify F1.9, NTF3
		Procedure: write more than the maximum file size. 
			0 bytes should be written since max size is already reached. 
		Expected output: 0
	*/

	//The file "/test2.txt" (fd1) was previously opened during test 13
	ret1 = writeFile(fd1, oversizedBuffer, MAX_FILE_SIZE + 1);
	if(ret1 == 0){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 19 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 19 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	//oversizedBuffer no longer used
	free(oversizedBuffer);

	/*
		Test ID: 20
		Objective: Verify F1.10
		Procedure: modify seek position of a file to point to its end.
		Expected output: 0
	*/

	//The file "/test2.txt" (fd1) was previously opened during test 13
	ret1 = lseekFile(fd1, 41, FS_SEEK_END );
	if(ret1 == 0){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 20 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 20 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}

	/*
		Test ID: 21
		Objective: Verify F1.10
		Procedure:  modify seek position using an invalid whence 
		Expected output: -1
	*/

	//The file "/test2.txt" (fd1) was previously opened during test 13
	//Whence are defined as constants but they are integers
	ret1 = lseekFile(fd1, 41, -1);
	if(ret1 == -1){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 21 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 21 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}

	/*
		Test ID: 22
		Objective: Verify F1.12 
		Procedure: add integrity to a file 
		Expected output: 0
	*/

	//The file "/test2.txt" (fd1) was previously opened during test 13
	closeFile(fd1);
	ret1 = includeIntegrity("/test2.txt");
	if(ret1 == 0){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 22 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 22 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}

	/*
		Test ID: 23
		Objective: Verify F1.13, NTF10
		Procedure: open a file with integrity
		Expected output: 1 (file descriptor available)
	*/
	//The inode number is used as file descriptor for the file, thus for "/test2.txt" fd = 1
	fd1 = openFileIntegrity("/test2.txt");
	if(fd1 == 1){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 23 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 23 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}

	/*
		Test ID: 24
		Objective: Verify NTF11
		Procedure: close a file with integrity using closeFile() 
		Expected output: -1
	*/
	//The file "/test2.txt" (fd1) was previously opened during test 23
	ret1 = closeFile(fd1);
	if(ret1 == -1){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 24 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 24 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}

	/*
		Test ID: 25
		Objective: Verify F1.14 
		Procedure: close a file with integrity using closeFileIntegrity() 
		Expected output: 0
	*/

	//The file "/test2.txt" (fd1) was previously opened during test 23
	ret1 = closeFileIntegrity(fd1);
	if(ret1 == 0){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 25 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 25 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}

	/*
		Test ID: 26
		Objective: Verify F1.16 
		Procedure: delete a symbolic link
		Expected output: 0
	*/

	ret1 = removeLn("/testLn");
	if(ret1 == 0){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 26 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 26 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}

	/*
		Test ID: 27
		Objective: Verify F1.16
		Procedure: delete a non existent link 
		Expected output: -1
	*/

	ret1 = removeLn("empty");
	if(ret1 == -1){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 27 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 27 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}

	/*
		Test ID: 28
		Objective: Verify F1.3 
		Procedure: unmount the file system while there are opened files
		Expected output: -1
	*/

	ret1 = unmountFS();
	if(ret1 == -1){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 28 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 28 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}

/*
		Test ID: 29
		Objective: Verify F1.3 
		Procedure: close all the opened files and unmount the file system
		Expected output: 0 0
	*/

	ret1 = closeAllFiles();
	ret2 = unmountFS();
	if(ret1 == 0 && ret2 == 0){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 29 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		printf("ret1 = %d    ret2 = %d\n", ret1, ret2);
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 29 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}

	/*
		Test ID: 30
		Objective: Verify F1.3
		Procedure: unmount a FS that was not mounted
		Expected output: -1
	*/

	ret1 = unmountFS();
	if(ret1 == -1){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 30 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 30 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}

	/*
		Test ID: 31
		Objective: Verify F1.3, NTF5 
		Procedure: mount again the unmounted FS. The metadata should be conserved (including, 
			among other things, the names of the files and the integrity values) 
		Expected output: 0 1 (file descriptor of the file) 
	*/
	ret1 = mountFS();
	fd1 = openFileIntegrity("/test2.txt"); 
	if(ret1 == 0 && fd1 == 1){
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 31 ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}else{
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST 31 ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	///////
	
	//All test completed successfully
	return 0;
}
