# Emvolution (fork)

A work-in-progress Wii game file replacement engine written for the BrainSlug game patching utility.

Tested and confirmed to be working on Mario Kart Wii PAL (RMCP).

The aim of this fork is to provide the ability to define which files to replace without needing to recompile
this module.

## How to use it ?

This module depends on `libfat.mod`, `libsd.mod` and `libfat-sd.mod`. You can compile them from
[Brainslug's repository](https://github.com/Chadderz121/brainslug-wii/tree/master/modules). You also need
to put DVD's symbols into `bslug/symbols` (you can find it from `symbols/dvd.xml`).

When module is installed you can begin to define each files should be replaced. By default,
all custom files are located to `sd:/custom/????/files` with `????` correspond to disc'ID (for example,
Mario Kart Wii PAL is `RMCP`, Super Smash Bros. Brawl NTSC-U is `RSBE`). Moreover, you need to create a TXT file at
`sd:/custom/????/replacements.txt` where each lines correspond to a file to replace. Example of `replacemements.txt`:
```
/Demo/Award.szs
/Scene/UI/Award.szs
/sound/revo_kart.brsar
```
I haven't yet tested module's behaviour with CRLF end lines and empty lines. Filepaths must begin with `/`.

For more informations, please look at original's [README](https://github.com/InvoxiPlayGames/emvolution/blob/master/README.md).
This version still at an early stage, not every DVD's functions are hooked and may not working properly. 

## Limitations

Since Brainslug discourage to use dynamic allocation, everything is static. So, you can't replace more than
512 files and filepath can't be longer than 256 characters.

You can change these restrictions from `replacements.h`.

## Module configuration

The following constants can be changed from `replacements.h` :
- `CUSTOM_FILES_DIRECTORY`: Location of custom files to use
- `CUSTOM_FILES_REPLACEMENTS_TXT`: Location of `replacements.txt`
- `MAX_FILE_COUNT`: Maximum allowed files to replace
- `MAX_NAME_LENGTH`: Maximum filepath's length

## Building

The process to build still the same from original repository :

1. [Install the latest devkitPPC and libogc.](https://devkitpro.org/wiki/Getting_Started)
2. Download the latest [BrainSlug](https://github.com/Chadderz121/brainslug-wii) source code and run `make install` to install the required files.
3. Run `make` inside the Emvolution project directory. It will produce `emvolution.mod` inside `bin` directory.

Now, you can install it into `bslug/modules`!