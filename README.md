# PS3 ISO Utilities
[![Downloads](https://img.shields.io/github/downloads/bucanero/ps3iso-utils/total.svg?maxAge=3600)](https://github.com/bucanero/ps3iso-utils/releases)
[![License](https://img.shields.io/github/license/bucanero/ps3iso-utils.svg)](./LICENSE)

Windows, Linux, and macOS builds of [Estwald's](https://github.com/Estwald) PS3 ISO utilities:

- [extractps3iso](#extractps3iso)
- [makeps3iso](#makeps3iso)
- [patchps3iso](#patchps3iso)
- [splitps3iso](#splitps3iso)

**Note:** Use `--help` or `/?` as parameter to see the usage information

## extractps3iso

Usage:

| Command | Description |
| :----- | :------ |
`extractps3iso`                           | interactive console input
`extractps3iso <ISO file>`                | default destination folder
`extractps3iso <ISO file> <pathfiles>`    | use `pathfiles` as destination folder
`extractps3iso -s <ISO file>`             | split big files (FAT32)
`extractps3iso -s <ISO file> <pathfiles>` | split big files (FAT32)

## makeps3iso

Usage:

| Command | Description |
| :----- | :------ |
`makeps3iso`                                     | interactive console input
`makeps3iso <pathfiles>`                         | default ISO name
`makeps3iso <pathfiles> <ISO file or folder>`    | file or folder destination
`makeps3iso -s <pathfiles>`                      | split files to 4GB
`makeps3iso -s <pathfiles> <ISO file or folder>` | split files to 4GB

## patchps3iso

Usage:

| Command | Description |
| :----- | :------ |
`patchps3iso`                       | interactive console input
`patchps3iso <ISO file>`            | default version (4.21)
`patchps3iso <ISO file> <version>`  | with version (4.21 to 4.60)

**Note:** `patchps3iso` can patch ISO split files (.iso.x or .ISO.x)

## splitps3iso

Usage:

| Command | Description |
| :----- | :------ |
`splitps3iso`                       | interactive console input
`splitps3iso <ISO file>`            | split ISO image (4Gb)
