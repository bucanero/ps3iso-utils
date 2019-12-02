# ps3iso utilities v1.9 (macOS)
macOS Mojave build of [Estwald's](https://github.com/Estwald) ps3iso utilities v1.9

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

NOTE 1: patchps3iso can patch ISO split files (.iso.x or .ISO.x)

NOTE 2: Use --help or /? as parameter to see the usage

