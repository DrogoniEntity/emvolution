#include <bslug.h>
#include <rvl/cache.h>
#include <io/fat-sd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <rvl/dvd.h>
#include "replacements.h"

BSLUG_MODULE_GAME("????");
BSLUG_MODULE_NAME("Emvolution");
BSLUG_MODULE_VERSION("1.0-fork");
BSLUG_MODULE_AUTHOR("InvoxiPlayGames, modded by DrogoniEntity");
BSLUG_MODULE_LICENSE("MIT");

/**
 * Turn it to "1" to toggle verbosity (for logging about file loading process).
 */
#define VERBOSE 0

/**
 * Flag to indicate if SD has been mounted.
 */
static bool _Initialized = false;

struct ReplaceEntry {
	char filename[MAX_NAME_LENGTH];
	int fd;
	FILE_STRUCT info;
};
static struct ReplaceEntry _ReplaceRegistry[MAX_FILE_COUNT];
static uint32_t _EntryCount;

/**
 * Formatted version of "CUSTOM_FILES_DIRECTORY".
 */
static char _CustomFilesRootPath[sizeof(CUSTOM_FILES_DIRECTORY) + 2];

static void InitializeSDAccess()
{
	_Initialized = true;
	_EntryCount = 0;
	
	#if VERBOSE
	printf("[Emvolution] Mounting SD card...\n");
	#endif
	if (SD_Mount() == 0)
	{
		// Fetching Disc ID and formatting "CUSTOM_FILES_DIRECTORY"...
		char discId[5];
		memcpy(&discId, (void *) 0x80000000, 4);
		discId[4] = '\0';
		sprintf(_CustomFilesRootPath, CUSTOM_FILES_DIRECTORY, discId);
		
		char txtFilePath[sizeof(CUSTOM_FILES_REPLACEMENTS_TXT) + 2];
		sprintf(txtFilePath, CUSTOM_FILES_REPLACEMENTS_TXT, discId);
		
		int index = 0;
		FILE_STRUCT fs;
		int fd = SD_open(&fs, txtFilePath, O_RDONLY);
		if (fd != -1)
		{
			#if VERBOSE
			printf("[Emvolution] Replacement file \"%s\" opened on %d\n", txtFilePath, fd);
			#endif
			char buffer[1024];
			ssize_t bytes_readed;
			
			int nameCharIndex = 0;
			while ((bytes_readed = SD_read(fd, buffer, 1024)) > 0)
			{
				if (index < MAX_FILE_COUNT)
				{
					for (int i = 0; i < bytes_readed; i++)
					{
						char byte = buffer[i];
						if (byte == '\r')
							continue;
						if (byte != '\n' && byte != '\0')
						{
							if (nameCharIndex < MAX_NAME_LENGTH)
								_ReplaceRegistry[index].filename[nameCharIndex] = byte;
							nameCharIndex++;
							if (nameCharIndex >= MAX_NAME_LENGTH)
								_ReplaceRegistry[index].filename[0] = '\0';
						}
						else
						{
							if (nameCharIndex < MAX_NAME_LENGTH)
							{
								_ReplaceRegistry[index].filename[nameCharIndex] = '\0';
								#if VERBOSE
								printf("[Emvolution] Custom file to inject: %s\n", _ReplaceRegistry[index].filename);
								#endif
								index++;
							}
							
							nameCharIndex = 0;
						}
					}
				}
			}
			if (index < MAX_FILE_COUNT && nameCharIndex < MAX_NAME_LENGTH)
			{
				_ReplaceRegistry[index].filename[nameCharIndex] = '\0';
				#if VERBOSE
				printf("[Emvolution] Custom file to inject: %s\n", _ReplaceRegistry[index].filename);
				#endif
				index++;
			}
			
			_EntryCount = (index >= MAX_FILE_COUNT) ? MAX_FILE_COUNT : index;
			SD_close(fd);
		}
		#if VERBOSE
		else
		{
			printf("[Emvolution] Replacements file \"%s\" open failed\n", txtFilePath);
		}
		#endif
	}
	#if VERBOSE
	else
	{
		printf("[Emvolution] SD Mount failed\n");
	}
	#endif
}

static bool DVDOpen_Hook(const char * path, DVDFileInfo * fileInfo)
{
	#if VERBOSE
    printf("[Emvolution] DVDOpen (path = %s)\n", path);
	#endif
	
    if (!_Initialized)
		InitializeSDAccess();
	
	for (int i = 0; i < _EntryCount; i++)
	{
		// "path" can begin with '/' or not, so let's checking in both case
		if (strcmp(path, _ReplaceRegistry[i].filename) == 0 || strcmp(path, &_ReplaceRegistry[i].filename[1]) == 0)
		{
			char new_path[sizeof(_CustomFilesRootPath) + strlen(path)];
			strcpy(new_path, _CustomFilesRootPath);
			strcat(new_path, path);
			
			int fd = SD_open(&(_ReplaceRegistry[i].info), new_path, O_RDONLY);
			if (fd != -1)
			{
				#if VERBOSE
				printf("[Emvolution] File \"%s\" (index = %d) opened on %d\n", new_path, i, fd);
				#endif
				_ReplaceRegistry[i].fd = fd;
				fileInfo->start = 0xFFFF0000 + i;
				fileInfo->filesize = _ReplaceRegistry[i].info.filesize;
				
				return true;
			}
		}
	}
	
	// Fallback
	return DVDOpen(path, fileInfo);
}
BSLUG_REPLACE(DVDOpen, DVDOpen_Hook);

static int32_t DVDConvertPathToEntrynum_Hook(const char * path) {
	#if VERBOSE
    printf("[Emvolution] DVDConvertPathToEntrynum (path = %s)\n", path);
	#endif
    if (!_Initialized)
		InitializeSDAccess();
	
	for (int i = 0; i < _EntryCount; i++)
	{
		if (strcmp(path, _ReplaceRegistry[i].filename) == 0 || strcmp(path, &_ReplaceRegistry[i].filename[1]) == 0)
			return 10000 + i;
	}
	
    return DVDConvertPathToEntrynum(path);
}
BSLUG_REPLACE(DVDConvertPathToEntrynum, DVDConvertPathToEntrynum_Hook);

static bool DVDFastOpen_Hook(int32_t entrynum, DVDFileInfo * fileInfo) {
	#if VERBOSE
    printf("[Emvolution] DVDFastOpen (entry = %d)\n", entrynum);
	#endif
    if (!_Initialized)
		InitializeSDAccess();
	
	if (entrynum >= 10000)
	{
		int i = entrynum - 10000;
		char * path = _ReplaceRegistry[i].filename;
		char new_path[sizeof(_CustomFilesRootPath) + strlen(path)];
		strcpy(new_path, _CustomFilesRootPath);
		strcat(new_path, path);
		
		int fd = SD_open(&(_ReplaceRegistry[i].info), new_path, O_RDONLY);
		if (fd != -1)
		{
			#if VERBOSE
			printf("[Emvolution] File \"%s\" (index = %d) opened on %d\n", new_path, i, fd);
			#endif
			_ReplaceRegistry[i].fd = fd;
			fileInfo->start = 0xFFFF0000 + i;
			fileInfo->filesize = _ReplaceRegistry[i].info.filesize;
			
			return true;
		}
	}
	
    return DVDFastOpen(entrynum, fileInfo);
}
BSLUG_REPLACE(DVDFastOpen, DVDFastOpen_Hook);

static bool DVDClose_Hook(DVDFileInfo * fileInfo)
{
	#if VERBOSE
	printf("[Emvolution] DVDClose (fileInfo->start = %u)\n", fileInfo->start);
	#endif
    if (fileInfo->start >= 0xFFFF0000)
	{
        int realentry = fileInfo->start - 0xFFFF0000;
        return (SD_close(_ReplaceRegistry[realentry].fd) == 0);
    }
	
    return DVDClose(fileInfo);
}
BSLUG_REPLACE(DVDClose, DVDClose_Hook);

static int DVDReadPrio_Hook(DVDFileInfo * fileInfo, void * buf, int len, int offset, int prio)
{
    if (fileInfo->start >= 0xFFFF0000)
	{
        int index = fileInfo->start - 0xFFFF0000;
		#if VERBOSE
        printf("[Emvolution] DVDReadPrio[READING] (fd = %d, index = %i, offset = %d, len = %d, prio = %d)\n", _ReplaceRegistry[index].fd, index, offset, len, prio);
		#endif
        SD_seek(_ReplaceRegistry[index].fd, offset, SEEK_SET);
        int bytes = SD_read(_ReplaceRegistry[index].fd, buf, len);
        DCFlushRange(buf, len);
		#if VERBOSE
        printf("[Emvolution] DVDReadPrio[READED] (fd = %d, index = %i) -> %d byte(s) readed\n", _ReplaceRegistry[index].fd, index, bytes);
		#endif
        return bytes;
    }
	
    return DVDReadPrio(fileInfo, buf, len, offset, prio);
}
BSLUG_REPLACE(DVDReadPrio, DVDReadPrio_Hook);