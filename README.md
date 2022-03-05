# PS3 ISO Utilities
[![Downloads](https://img.shields.io/github/downloads/bucanero/ps3iso-utils/total.svg?maxAge=3600)](https://github.com/bucanero/ps3iso-utils/releases)
[![Build binaries](https://github.com/bucanero/ps3iso-utils/actions/workflows/build.yml/badge.svg)](https://github.com/bucanero/ps3iso-utils/actions/workflows/build.yml)
[![Build Windows binaries](https://github.com/bucanero/ps3iso-utils/actions/workflows/build-win.yml/badge.svg)](https://github.com/bucanero/ps3iso-utils/actions/workflows/build-win.yml)
[![License](https://img.shields.io/github/license/bucanero/ps3iso-utils.svg)](./LICENSE)

Windows, Linux, macOS, and [Docker](#docker) builds of [Estwald's](https://github.com/Estwald) PS3 ISO utilities:

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

# Docker

It is possible to run these commands through a Docker or Podman container. The image size is less than 7MB.

1. Download the `Dockerfile`

2. Build the Docker image:

```
$ docker build -t ps3iso-utils .
```

3. Append the following functions to your `.bashrc`:

```
extractps3iso () {
	docker run --rm -v "$PWD":/tmp localhost/ps3iso-utils extractps3iso "$@"
}

makeps3iso () {
	docker run --rm -v "$PWD":/tmp localhost/ps3iso-utils makeps3iso "$@"
}

patchps3iso () {
	docker run --rm -v "$PWD":/tmp localhost/ps3iso-utils patchps3iso "$@"
}

splitps3iso () {
	docker run --rm -v "$PWD":/tmp localhost/ps3iso-utils splitps3iso "$@"
}

```

4. Source your `.bashrc` or close and reopen your terminal:

```
$ source ~/.bashrc
```

5. Run the program

Examples:

```
$ splitps3iso --help

SPLITPS3ISO (c) 2021, Bucanero

Usage:

    splitps3iso                       -> input data from the program
    splitps3iso <ISO file>            -> split ISO image (4Gb)
```

```
$ makeps3iso BLUS12345

MAKEPS3ISO (c) 2013, Estwald (Hermes)


</>     
  -> PS3_DISC.SFB LBA 58 size 1 KB
</PS3_GAME>                  
  -> ICON0.PNG LBA 59 size 46 KB
...
```

