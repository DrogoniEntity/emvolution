#ifndef __REPLACEMENTS_H__
#define __REPLACEMENTS_H__

/**
 * Define where files to replacement should be located.
 *
 * "%s" will be replaced with game's ID (always encoded on 4 bytes).
 */
#define CUSTOM_FILES_DIRECTORY "sd:/custom/%s/files"

/**
 * File to indicate which files should be replaced.
 *
 * "%s" will be replaced with game's ID (always encoded on 4 bytes).
 */
#define CUSTOM_FILES_REPLACEMENTS_TXT "sd:/custom/%s/replacements.txt"

/**
 * Max allowed files to replace.
 */
#define MAX_FILE_COUNT 512

/**
 * Max file's name.
 */
#define MAX_NAME_LENGTH 256

#endif