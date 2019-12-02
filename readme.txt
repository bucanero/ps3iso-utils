Usage:

extractps3iso                           -> input datas from the program
extractps3iso <ISO file>                -> default destination folder
extractps3iso <ISO file> <pathfiles>    -> pathfiles is destination folder
extractps3iso -s <ISO file>             -> split big files (FAT32)
extractps3iso -s <ISO file> <pathfiles> -> split big files (FAT32)

 
makeps3iso                                     -> input datas from the program
makeps3iso <pathfiles>                         -> default ISO name
makeps3iso <pathfiles> <ISO file or folder>    -> file or folder destination
makeps3iso -s <pathfiles>                      -> split files to 4GB
makeps3iso -s <pathfiles> <ISO file or folder> -> split files to 4GB

patchps3iso                       -> input datas from the program
patchps3iso <ISO file>            -> default version (4.21)
patchps3iso <ISO file> <version>  -> with version (4.21 to 4.60)

NOTE1: patchps3iso can patch ISO split files (.iso.x or .ISO.x)

NOTE2: Use --help or /? as parameter to see the usage

