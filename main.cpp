// Notes:
// Similarly to Frederick Raynal's HQR files, PAK files also have a header containing offsets to files.
// Additionally, each file has a header of 0x10 (16 bytes).
// These bytes contain: (unknown), compressed size, real size, and compression type.

// Libraries
#include <iostream>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

// Macros
#define MAX_FILES 1000
#define MAX_BYTES 65536

// Enums

//
// Structs.
//
typedef struct
	{
	int unknown;
	int originalSize;
	int compressedSize;
	int compressionType;
	} addressHeader;


// Variables
FILE   *packedFile;
DIR    *d;

struct dirent *dir;

int    entries;
int    totalBytes;

int  headerOffset;

char externalFileNames[MAX_BYTES][MAX_FILES];
char externalFileBytes[MAX_BYTES][MAX_FILES];

int  externalFileSizes[MAX_FILES];
int  externalFileOffsets[MAX_FILES];

int zero = 0x0000;

// Functions
size_t getFileSize(const char* filename)
	{
    struct stat st;
    
    if (stat(filename, &st) != 0)
		{
        return 0;
    	}
    
    return st.st_size;   
	};

int calculateFileOffset(int index)
	{
	int amount = headerOffset;
	
	for (int j = 0; j < index; j++)
		{
		amount += (externalFileSizes[j]+16);
		};
	
	return amount;
	};

void packBodies()
	{
	printf("\nReading body files... \n");
	
	d = opendir((const char*)"bodies/");
	
	if (d)
		{
		while ((dir = readdir(d)) != NULL)
			{
			if (strncmp(dir->d_name, "z", 1) == 0)
				{
				// Print the name of the external file.
				printf("Found '%s'. \n", dir->d_name);
				
				// Save the name of the external file.
				strcat(externalFileNames[entries], "bodies/");
				strcat(externalFileNames[entries], dir->d_name);
				
				// Get the size of the external file.
				externalFileSizes[entries] = getFileSize(externalFileNames[entries]);
				
				// Update the size of the packed file.
				totalBytes += externalFileSizes[entries]; // entries
				
				// The external file's contents will be used in a future entry.
				entries += 1;
				};
			};
		
		closedir(d);
		};
	
	printf("%d entries created. \n", entries);
	
	printf("\nPacking bodies... \n");
	
	packedFile = fopen("LISTBODY.PAK", "wb");
	
	// Count all entries plus an extra integer containing the size of the entire file.
	headerOffset = (entries * sizeof(int)) + sizeof(int);
	
	// Calculate all offsets from external files.
	for (int i = 0; i < entries; i++)
		externalFileOffsets[i] = calculateFileOffset(i);
	
	// Stub offsets of the entire file.
	for (int i = 0; i < entries; i++) // entries
		{
		totalBytes += sizeof(int);
		fwrite(&totalBytes, sizeof(int), 0x1, packedFile);
		};
	
	// Size of entire file.
	totalBytes += sizeof(int);
	for (int i = 0; i < entries; i++)
		{
		totalBytes += 16;
		};
	fwrite(&totalBytes, sizeof(int), 0x1, packedFile);
	
	// External file data - packed into the entire file.
	for (int i = 0; i < entries; i++)
		{
		// Setup file.
		FILE *currentFile;
		
		// Setup file header.
		addressHeader thisHeader;
		
		thisHeader.unknown =         0x00;
		thisHeader.originalSize =    externalFileSizes[i];
		thisHeader.compressedSize =  externalFileSizes[i];
		thisHeader.compressionType = 0x00;
		
		// Fill file header data.
		fwrite(&thisHeader.unknown,         sizeof(int), 0x1, packedFile);
		fwrite(&thisHeader.originalSize,    sizeof(int), 0x1, packedFile);
		fwrite(&thisHeader.compressedSize,  sizeof(int), 0x1, packedFile);
		fwrite(&thisHeader.compressionType, sizeof(int), 0x1, packedFile);
		
		// Open the file.
		currentFile = fopen(externalFileNames[i], "rb");
		
		printf("Packed entry %d. \n", i);
		
		// Uncompressed file data.
		for (int j = 0; j < externalFileSizes[i]; j++)
			{
			// Transfer the bytes from the current file to the packed file.
			fread(&externalFileBytes[j][i],  sizeof(char), 0x1, currentFile);
			fwrite(&externalFileBytes[j][i], sizeof(char), 0x1, packedFile);
			};
		
		// Close the file.
		fclose(currentFile);
		};
	
	// Go back to the beginning of the entire file.
	fseek(packedFile, 0x00, SEEK_SET);
	
	// Mark 'fake' offset.
	fwrite(&zero, sizeof(int), 0x1, packedFile);
	
	// Create offsets for the entire file.
	for (int i = 0; i < entries; i++)
		{
		fwrite(&externalFileOffsets[i], sizeof(int), 0x1, packedFile);
		};
	
	fclose(packedFile);
	
	printf("Finished packing! \n");
	};

int main()
	{
	char option;
	
	printf("AITD (PAK Packer) - By Quilt \n");
	printf("	This program will let you change almost everything in AITD. \n");
	printf("	However, it has only been tested for replacing bodies. \n");
	printf("	Future options are soon to come. \n");
	
	printf("\nWhat would you like to pack? \n");
	printf("1 - Pack to custom bodies. \n");
	
	scanf("%c", &option);
	
	if (option == '1')
		{
		fflush(stdin); // Clear the input.
		packBodies();
		};
	
	return 0;
	};
