/* 
    (c) 2013 Estwald/Hermes <www.elotrolado.net>

    EXTRACTPS3ISO is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    EXTRACTPS3ISO is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    apayloadlong with EXTRACTPS3ISO.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

#if defined (__MSVCRT__)
#define stat _stati64
#endif

#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long

static int get_input_char()
{
    char c = getchar();
    char c2 = c;
    while (c != '\n' && c != EOF)
     	c = getchar();
    return c2;
}

#ifdef __MSVCRT__
#include <windows.h>
#include <winbase.h>

int verbose = 1;

static u64 get_disk_free_space(char *path)
{
 
    DWORD SectorsPerCluster, BytesPerSector, NumberOfFreeClusters, TotalNumberOfClusters;
    
    if(!GetDiskFreeSpace(path, &SectorsPerCluster, &BytesPerSector, &NumberOfFreeClusters, &TotalNumberOfClusters))
        return (u64) (-1LL);

    return ((u64) BytesPerSector) * ((u64) SectorsPerCluster) * ((u64) NumberOfFreeClusters);

}
#elif __unix__ 
#include <sys/statvfs.h>

static u64 get_disk_free_space(char *path)
{
    
    struct statvfs svfs;
    
    if(statvfs((const char *) path, &svfs)!=0)
        return (u64) (-1LL);

    return ( ((u64)svfs.f_bsize * svfs.f_bfree));


}

#else
#error "include here your own method to get free disk space or remove this line"

static u64 get_disk_free_space(char *path)
{

    return (u64) (-1LL);

}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////


#define ISODCL(from, to) (to - from + 1)

static int isonum_731 (unsigned char * p)
{
	return ((p[0] & 0xff)
		| ((p[1] & 0xff) << 8)
		| ((p[2] & 0xff) << 16)
		| ((p[3] & 0xff) << 24));
}


static int isonum_733 (unsigned char * p)
{
	return (isonum_731 (p));
}


static int isonum_721 (char * p)
{
	return ((p[0] & 0xff) | ((p[1] & 0xff) << 8));
}

struct iso_primary_descriptor {
	unsigned char type			[ISODCL (  1,   1)]; /* 711 */
	unsigned char id				[ISODCL (  2,   6)];
	unsigned char version			[ISODCL (  7,   7)]; /* 711 */
	unsigned char unused1			[ISODCL (  8,   8)];
	unsigned char system_id			[ISODCL (  9,  40)]; /* aunsigned chars */
	unsigned char volume_id			[ISODCL ( 41,  72)]; /* dunsigned chars */
	unsigned char unused2			[ISODCL ( 73,  80)];
	unsigned char volume_space_size		[ISODCL ( 81,  88)]; /* 733 */
	unsigned char unused3			[ISODCL ( 89, 120)];
	unsigned char volume_set_size		[ISODCL (121, 124)]; /* 723 */
	unsigned char volume_sequence_number	[ISODCL (125, 128)]; /* 723 */
	unsigned char logical_block_size		[ISODCL (129, 132)]; /* 723 */
	unsigned char path_table_size		[ISODCL (133, 140)]; /* 733 */
	unsigned char type_l_path_table		[ISODCL (141, 144)]; /* 731 */
	unsigned char opt_type_l_path_table	[ISODCL (145, 148)]; /* 731 */
	unsigned char type_m_path_table		[ISODCL (149, 152)]; /* 732 */
	unsigned char opt_type_m_path_table	[ISODCL (153, 156)]; /* 732 */
	unsigned char root_directory_record	[ISODCL (157, 190)]; /* 9.1 */
	unsigned char volume_set_id		[ISODCL (191, 318)]; /* dunsigned chars */
	unsigned char publisher_id		[ISODCL (319, 446)]; /* achars */
	unsigned char preparer_id		[ISODCL (447, 574)]; /* achars */
	unsigned char application_id		[ISODCL (575, 702)]; /* achars */
	unsigned char copyright_file_id		[ISODCL (703, 739)]; /* 7.5 dchars */
	unsigned char abstract_file_id		[ISODCL (740, 776)]; /* 7.5 dchars */
	unsigned char bibliographic_file_id	[ISODCL (777, 813)]; /* 7.5 dchars */
	unsigned char creation_date		[ISODCL (814, 830)]; /* 8.4.26.1 */
	unsigned char modification_date		[ISODCL (831, 847)]; /* 8.4.26.1 */
	unsigned char expiration_date		[ISODCL (848, 864)]; /* 8.4.26.1 */
	unsigned char effective_date		[ISODCL (865, 881)]; /* 8.4.26.1 */
	unsigned char file_structure_version	[ISODCL (882, 882)]; /* 711 */
	unsigned char unused4			[ISODCL (883, 883)];
	unsigned char application_data		[ISODCL (884, 1395)];
	unsigned char unused5			[ISODCL (1396, 2048)];
};

struct iso_directory_record {
	unsigned char length			[ISODCL (1, 1)]; /* 711 */
	unsigned char ext_attr_length		[ISODCL (2, 2)]; /* 711 */
	unsigned char extent			[ISODCL (3, 10)]; /* 733 */
	unsigned char size			[ISODCL (11, 18)]; /* 733 */
	unsigned char date			[ISODCL (19, 25)]; /* 7 by 711 */
	unsigned char flags			[ISODCL (26, 26)];
	unsigned char file_unit_size		[ISODCL (27, 27)]; /* 711 */
	unsigned char interleave			[ISODCL (28, 28)]; /* 711 */
	unsigned char volume_sequence_number	[ISODCL (29, 32)]; /* 723 */
	unsigned char name_len		[1]; /* 711 */
	unsigned char name			[1];
};

struct iso_path_table{
	unsigned char  name_len[2];	/* 721 */
	char extent[4];		/* 731 */
	char  parent[2];	/* 721 */
	char name[1];
};



#define SWAP16(x) ((((u16)(x))>>8) | ((x) << 8))

static void UTF16_to_UTF8(u16 *stw, u8 *stb)
{
    while(SWAP16(stw[0])) {
        if((SWAP16(stw[0]) & 0xFF80) == 0) {
            *(stb++) = SWAP16(stw[0]) & 0xFF;   // utf16 00000000 0xxxxxxx utf8 0xxxxxxx
        } else if((SWAP16(stw[0]) & 0xF800) == 0) { // utf16 00000yyy yyxxxxxx utf8 110yyyyy 10xxxxxx
            *(stb++) = ((SWAP16(stw[0])>>6) & 0xFF) | 0xC0; *(stb++) = (SWAP16(stw[0]) & 0x3F) | 0x80;
        } else if((SWAP16(stw[0]) & 0xFC00) == 0xD800 && (SWAP16(stw[1]) & 0xFC00) == 0xDC00 ) { // utf16 110110ww wwzzzzyy 110111yy yyxxxxxx (wwww = uuuuu - 1) 
                                                                             // utf8 1111000uu 10uuzzzz 10yyyyyy 10xxxxxx  
            *(stb++)= (((SWAP16(stw[0]) + 64)>>8) & 0x3) | 0xF0; *(stb++)= (((SWAP16(stw[0])>>2) + 16) & 0x3F) | 0x80; 
            *(stb++)= ((SWAP16(stw[0])>>4) & 0x30) | 0x80 | ((SWAP16(stw[1])<<2) & 0xF); *(stb++)= (SWAP16(stw[1]) & 0x3F) | 0x80;
            stw++;
        } else { // utf16 zzzzyyyy yyxxxxxx utf8 1110zzzz 10yyyyyy 10xxxxxx
            *(stb++)= ((SWAP16(stw[0])>>12) & 0xF) | 0xE0; *(stb++)= ((SWAP16(stw[0])>>6) & 0x3F) | 0x80; *(stb++)= (SWAP16(stw[0]) & 0x3F) | 0x80;
        } 
        
        stw++;
    }
    
    *stb= 0;
}

static void fixpath(char *p)
{
    u8 * pp = (u8 *) p;

    if(*p == '"') {
        p[strlen(p) -1] = 0;
        memcpy(p, p + 1, strlen(p));
    }

    #ifdef __CYGWIN__
    if(p[0]!=0 && p[1] == ':') {
        p[1] = p[0];
        memmove(p + 9, p, strlen(p) + 1);
        memcpy(p, "/cygdrive/", 10);
    }
    #endif

    while(*pp) {
        if(*pp == '"') {*pp = 0; break;}
        else
        if(*pp == '\\') *pp = '/';
        else
        if(*pp > 0 && *pp < 32) {*pp = 0; break;}
        pp++;
    }

}

#define MAX_ISO_PATHS 4096

typedef struct {
    int parent;
    char *name;

} _directory_iso2;

static _directory_iso2 *directory_iso2 = NULL;

typedef struct {
    u32 size;
    char path[0x420];

} _split_file;

static _split_file split_file[64];

static void get_iso_path(char *path, int indx) 
{
    char aux[0x420];

    path[0] = 0;

    if(!indx) {path[0] = '/'; path[1] = 0; return;}

    while(1) {
        strcpy(aux, directory_iso2[indx].name);
        strcat(aux, path);
        strcpy(path, aux);
       
        indx = directory_iso2[indx].parent - 1;
        if(indx == 0) break;     
    }

}


static FILE *fp_split = NULL;
static FILE *fp_split0 = NULL;

static int split_index = 0;

static int read_split(u64 position, u8 *mem, int size)
{

    int n;

    if(!split_file[1].size) {

        if(!fp_split0) fp_split0 = fopen(split_file[0].path, "rb");
        if(!fp_split0) return -666;

        if(fseeko64(fp_split0, position, SEEK_SET)<0) {
            printf("Error!: in ISO file fseek\n\n");
            return -668;
        }

        if(fread((void *) mem, 1, size, fp_split0) != size) return -667;
        return 0;
    }

    u64 relpos0 = 0;
    u64 relpos1 = 0;

    for(n = 0; n < 64; n++){
        if(!split_file[n].size) return -669;
        if(position < (relpos0 + (u64) split_file[n].size)) {
            relpos1 = relpos0 + (u64) split_file[n].size;
            break;
        }

        relpos0 += split_file[n].size;
    }
    
    if(fp_split == NULL) split_index = 0;

    if(n == 0) {
        if(split_index && fp_split) {fclose(fp_split); fp_split = NULL;}
        split_index = 0;
        fp_split = fp_split0;

    } else {

        if(n != split_index) {
            if(split_index && fp_split) {fclose(fp_split); fp_split = NULL;}

            split_index = n;

            fp_split = fopen(split_file[split_index].path, "rb");
            if(!fp_split) return -666;

        }
    }

    //int cur = lba / SPLIT_LBA;
    //int cur2 = (lba + sectors) / SPLIT_LBA;

    if(fseeko64(fp_split, (position - relpos0), SEEK_SET)<0) {
        printf("Error!: in ISO file fseek\n\n");
        return -668;
    }

    if(position >= relpos0 && (position + size) < relpos1) {

        if(fread((void *) mem, 1, (int) size, fp_split) != size) return -667;
        return 0;
    }

    int lim = (int) (relpos1 - position);

    if(fread((void *) mem, 1, (int) lim, fp_split) != lim) return -667;

    mem += lim; size-= lim;

    if(split_index && fp_split) {fclose(fp_split); fp_split = NULL;}

    split_index++;

    fp_split = fopen(split_file[split_index].path, "rb");
    if(!fp_split) return -666;

    if(fread((void *) mem, 1, (int) size, fp_split) != size) return -667;

    return 0;
}

static int split_files = 0;

int main(int argc, const char* argv[])
{

    struct stat s;
    int n;
    int a = 0;
    int arg_split = 'n';

    char path1[0x420];
    char path2[0x420];
    int len_path2;

    FILE *fp2 = NULL;
    u8 *sectors = NULL;
    u8 *sectors2 = NULL;
    u8 *sectors3 = NULL;

    static char string[0x420];
    static char string2[0x420];
    static u16 wstring[1024];

    struct iso_primary_descriptor sect_descriptor;
    struct iso_directory_record * idr;
    int idx = -1;

    directory_iso2 = NULL;

    fp_split = NULL;
    fp_split0 = NULL;
    split_index = 0;
    split_files = 0; 

    clock_t t_start, t_finish;


    // libc test
    if(sizeof(s.st_size) != 8) {

        printf("Error!: stat st_size must be a 64 bit number!  (size %i)\n\nPress ENTER key to exit\n\n", sizeof(s.st_size));
        get_input_char();
        return -1;
    }

    if(argc > 1 && (!strcmp(argv[1], "/?") || !strcmp(argv[1], "--help"))) {

        printf("\nEXTRACTPS3ISO (c) 2013, Estwald (Hermes)\n\n");
    
        printf("%s", "Usage:\n\n"
               "    extractps3iso                           -> input datas from the program\n"
               "    extractps3iso <ISO file>                -> default destination folder\n"
               "    extractps3iso <ISO file> <pathfiles>    -> pathfiles is destination folder\n"
               "    extractps3iso -s <ISO file>             -> split big files (FAT32)\n"
               "    extractps3iso -s <ISO file> <pathfiles> -> split big files (FAT32)\n");
               
        return 0;
    }

    if(argc > 1 && (!strcmp(argv[a + 1], "-s") || !strcmp(argv[a + 1], "-S"))) {a++; arg_split = 'y';}
    if(argc > 1 && (!strcmp(argv[a + 1], "-p0") || !strcmp(argv[a + 1], "-P0"))) {a++; verbose = 0;}
    if(argc > 1 && (!strcmp(argv[a + 1], "-s") || !strcmp(argv[a + 1], "-S"))) {a++; arg_split = 'y';}

    if(verbose) printf("\nEXTRACTPS3ISO (c) 2013, Estwald (Hermes)\n\n");

    if(argc == 1) {
        printf("Enter PS3 ISO to extract:\n");
        if(fgets(path1, 0x420, stdin)==0) {
            printf("Error Input PS3 ISO!\n\nPress ENTER key to exit\n"); get_input_char();return -1;
        }
        printf("\n");
    } else {if(argc >= (2 + a)) strcpy(path1, argv[1 + a]); else path1[0] = 0;}

    if(path1[0] == 0) {
         printf("Error: ISO file don't exists!\n\nPress ENTER key to exit\n"); get_input_char();return -1;
    }

    fixpath(path1);

    n = strlen(path1);


    if(n >= 4 && (!strcmp(&path1[n - 4], ".iso") || !strcmp(&path1[n - 4], ".ISO"))) {

        sprintf(split_file[0].path, "%s", path1);
        if(stat(split_file[0].path, &s)<0) {
            printf("Error: ISO file don't exists!\n\nPress ENTER key to exit\n"); get_input_char();return -1;

        }

        split_file[0].size = s.st_size;
        split_file[1].size = 0; // split off
       
    } else if(n >= 6 && (!strcmp(&path1[n - 6], ".iso.0") || !strcmp(&path1[n - 6], ".ISO.0"))) {

        int m;

        for(m = 0; m < 64; m++) {
            strcpy(string2, path1);
            string2[n - 2] = 0; 
            sprintf(split_file[m].path, "%s.%i", string2, m);
            if(stat(split_file[m].path, &s)<0) break;
            split_file[m].size = s.st_size;
        }

        for(; m < 64; m++) {
            split_file[m].size = 0;
        }

       
    } else {
        printf("Error: file must be with .iso, .ISO .iso.0 or .ISO.0 extension\n\nPress ENTER key to exit\n"); get_input_char();return -1;
    }

    path2[0] = 0;
    if(argc == 1) {
        printf("Enter Path Folder to Extract:\n");
        if(fgets(path2, 0x420, stdin)==0) {
            printf("Error Input Path Folder to Extract!\n\nPress ENTER key to exit\n"); get_input_char();return -1;
        }
    } else {if(argc >= (3 + a)) strcpy(path2, argv[2 + a]); else path2[0] = 0;}

    fixpath(path2);

    if(path2[0] == 0) {
        strcpy(path2, path1);

        n= strlen(path2);

        if(!strcmp(&path2[n - 2], ".0")) path2[n - 6] = 0; else path2[n - 4] = 0;
        
        fixpath(path2);
  
    } else if(stat(path2, &s)==0) {

        strcat(path2, "/");

        char * o = strrchr(path1, '/');
        if(!o)
            strcat(path2, path1);
        else {
            strcat(path2, o + 1);
        }

        n= strlen(path2);

        if(!strcmp(&path2[n - 2], ".0")) path2[n - 6] = 0; else path2[n - 4] = 0;
        
        fixpath(path2);

        //printf("sd %s\n", path2);

    }

    if(argc < 2) printf("\nSplit 4GB files? (y/n):\n");
    
    split_files = (arg_split== 'y' || argc >= 2) ? arg_split : get_input_char();

    if(split_files == 'y' || split_files == 'Y') split_files = 1; else split_files = 0;

    if(split_files && verbose) printf("Using Split File Mode\n");

    if(verbose) printf("\n");

    len_path2 = strlen(path2);

    #if defined (__MSVCRT__)
    mkdir(path2); // make directory
    #else
    mkdir(path2, 0766); // make directory
    #endif

    u64 avail = get_disk_free_space(path2);

    FILE *fp = fopen(path1, "rb");
    if(!fp) {
        printf("Error!: Cannot open ISO file\n\nPress ENTER key to exit\n\n");
        get_input_char();
        return -1;
    }

    t_start = clock();

    
    if(fseeko64(fp, 0x8800, SEEK_SET)<0) {
        printf("Error!: in sect_descriptor fseek\n\n");
        goto err;
    }

    if(fread((void *) &sect_descriptor, 1, 2048, fp) != 2048) {
        printf("Error!: reading sect_descriptor\n\n");
        goto err;
    }

    if(!(sect_descriptor.type[0] == 2 && !strncmp((void *) &sect_descriptor.id[0], "CD001",5))) {
        printf("Error!: UTF16 descriptor not found\n\nPress ENTER key to exit\n\n");
        goto err;
    }

    u32 toc = isonum_733(&sect_descriptor.volume_space_size[0]);

    if((((u64)toc) * 2048ULL) > (avail - 0x100000ULL)) {
        printf("Error!: Insufficient Disk Space in Destination\n");
        goto err;
    }

    u32 lba0 = isonum_731(&sect_descriptor.type_l_path_table[0]); // lba
    u32 size0 = isonum_733(&sect_descriptor.path_table_size[0]); // tamaño
    //printf("lba0 %u size %u %u\n", lba0, size0, ((size0 + 2047)/2048) * 2048);
    
    if(fseeko64(fp, lba0 * 2048, SEEK_SET)<0) {
        printf("Error!: in path_table fseek\n\n");
        goto err;
    }

    directory_iso2 = malloc((MAX_ISO_PATHS + 1) * sizeof(_directory_iso2));

    if(!directory_iso2) {
        printf("Error!: in directory_is malloc()\n\n");
        goto err;
    }

    memset(directory_iso2, 0, (MAX_ISO_PATHS + 1) * sizeof(_directory_iso2));
 
    sectors = malloc(((size0 + 2047)/2048) * 2048);

    if(!sectors) {
        printf("Error!: in sectors malloc()\n\n");
        goto err;
    }

    sectors2 = malloc(2048 * 2);

    if(!sectors2) {
        printf("Error!: in sectors2 malloc()\n\n");
        goto err;
    }

    sectors3 = malloc(128 * 2048);

    if(!sectors3) {
        printf("Error!: in sectors3 malloc()\n\n");
        goto err;
    }

    if(fread((void *) sectors, 1, size0, fp) != size0) {
        printf("Error!: reading path_table\n\n");
        goto err;
    }


    u32 p = 0;

    string2[0] = 0;

    fp_split = NULL;
    fp_split0 = NULL;

    split_index = 0;


    idx = 0;

    directory_iso2[idx].name = NULL;

    if(!verbose) printf("\rPercent done: %u%% \r", 0);

    u32 flba = 0;

    while(p < size0) {

        u32 lba;

        u32 snamelen = isonum_721((void *) &sectors[p]);
        if(snamelen == 0) p= ((p/2048) * 2048) + 2048;
        p+=2;
        lba = isonum_731(&sectors[p]);
        p+=4;
        u32 parent =isonum_721((void *) &sectors[p]);
        p+=2;

        memset(wstring, 0, 512 * 2);
        memcpy(wstring, &sectors[p], snamelen);
        
        UTF16_to_UTF8(wstring, (u8 *) string);

        if(idx >= MAX_ISO_PATHS){
            printf("Too much folders (max %i)\n\n", MAX_ISO_PATHS);
            goto err;
        }

        directory_iso2[idx].name = malloc(strlen(string) + 2);
        if(!directory_iso2[idx].name) {
            printf("Error!: in directory_iso2.name malloc()\n\n");
            goto err;
        }

        strcpy(directory_iso2[idx].name, "/");
        strcat(directory_iso2[idx].name, string);
        
        directory_iso2[idx].parent = parent;
        
        get_iso_path(string2, idx);

        strcat(path2, string2);

        #if defined (__MSVCRT__)
        mkdir(path2); // make directory
        #else
        mkdir(path2, 0766); // make directory
        #endif

        path2[len_path2] = 0;
     
        if(verbose) printf("</%s>\n", string);
   
        u32 file_lba = 0;
        u64 file_size = 0;

        char file_aux[0x420];

        file_aux[0] = 0;

        int q2 = 0;
        int size_directory = 0;

        while(1) {

            if(fseeko64(fp, ((u64) lba) * 2048ULL, SEEK_SET)<0) {
                printf("Error!: in directory_record fseek\n\n");
                goto err;
            }

            memset(sectors2 + 2048, 0, 2048);

            if(fread((void *) sectors2, 1, 2048, fp) != 2048) {
                printf("Error!: reading directory_record sector\n\n");
                goto err;
            }

            int q = 0;
            
            if(q2 == 0) {
                idr = (struct iso_directory_record *) &sectors2[q];
                if((int) idr->name_len[0] == 1 && idr->name[0]== 0 && lba == isonum_731((void *) idr->extent) && idr->flags[0] == 0x2) {
                    size_directory = isonum_733((void *) idr->size);
                 
                } else {
                    printf("Error!: Bad first directory record! (LBA %i)\n\n", lba);
                    goto err;
                }
            }

            int signal_idr_correction = 0;

            while(1) {

               
                if(signal_idr_correction) {
                    signal_idr_correction = 0;
                    q-= 2048; // sector correction
                    // copy next sector to first
                    memcpy(sectors2, sectors2 + 2048, 2048);
                    memset(sectors2 + 2048, 0, 2048);
                    lba++;

                    q2 += 2048;

                }

                if(q2 >= size_directory) goto end_dir_rec;
               
                idr = (struct iso_directory_record *) &sectors2[q];

                if(idr->length[0]!=0 && (idr->length[0] + q) > 2048) {

                    printf("Warning! Entry directory break the standard ISO 9660\n\nPress ENTER key\n\n");
                    get_input_char();

                    if(fseeko64(fp, lba * 2048 + 2048, SEEK_SET)<0) {
                        printf("Error!: in directory_record fseek\n\n");
                        goto err;
                    }

                    if(fread((void *) (sectors2 + 2048), 1, 2048, fp) != 2048) {
                        printf("Error!: reading directory_record sector\n\n");
                        goto err;
                    }

                    signal_idr_correction = 1;

                }

                if(idr->length[0] == 0 && (2048 - q) > 255) goto end_dir_rec;

                if((idr->length[0] == 0 && q != 0) || q == 2048)  { 
                    
                    lba++;
                    q2 += 2048;

                    if(q2 >= size_directory) goto end_dir_rec;

                    if(fseeko64(fp, (((u64) lba) * 2048ULL), SEEK_SET)<0) {
                        printf("Error!: in directory_record fseek\n\n");
                        goto err;
                    }

                    if(fread((void *) (sectors2), 1, 2048, fp) != 2048) {
                        printf("Error!: reading directory_record sector\n\n");
                        goto err;
                    }
                    memset(sectors2 + 2048, 0, 2048);

                    q = 0;
                    idr = (struct iso_directory_record *) &sectors2[q];

                    if(idr->length[0] == 0 || ((int) idr->name_len[0] == 1 && !idr->name[0])) goto end_dir_rec;
                    
                }

                if((int) idr->name_len[0] > 1 && idr->flags[0] != 0x2 &&
                    idr->name[idr->name_len[0] - 1]== '1' && idr->name[idr->name_len[0] - 3]== ';') { // skip directories
                    
                    memset(wstring, 0, 512 * 2);
                    memcpy(wstring, idr->name, idr->name_len[0]);
                
                    UTF16_to_UTF8(wstring, (u8 *) string); 

                    if(file_aux[0]) {
                        if(strcmp(string, file_aux)) {
    
                            printf("Error!: in batch file %s\n\nPress ENTER key to exit\n\n", file_aux);
                            goto err;
                        }

                        file_size += (u64) (u32) isonum_733(&idr->size[0]);
                        if(idr->flags[0] == 0x80) {// get next batch file
                            q+= idr->length[0]; 
                            continue;
                        } 

                        file_aux[0] = 0; // stop batch file

                    } else {

                        file_lba = isonum_733(&idr->extent[0]);
                        file_size = (u64) (u32) isonum_733(&idr->size[0]);
                        if(idr->flags[0] == 0x80) {
                            strcpy(file_aux, string);
                            q+= idr->length[0];
                            continue;  // get next batch file
                        }
                    }

                    int len = strlen(string);

                    string[len - 2] = 0; // break ";1" string
                    
                    len = strlen(string2);
                    strcat(string2, "/");
                    strcat(string2, string);

                    if(verbose) {
                        if(file_size < 1024ULL) 
                            printf("  -> %s LBA %u size %u Bytes\n", string, file_lba, (u32) file_size);
                        else if(file_size < 0x100000LL) 
                            printf("  -> %s LBA %u size %u KB\n", string, file_lba, (u32) (file_size/1024));
                        else
                            printf("  -> %s LBA %u size %u MB\n", string, file_lba, (u32) (file_size/0x100000LL));
                    }

                    //printf("f %s\n", string2);

                    // writing procedure
                 
                    strcat(path2, string2);

                    int use_split = 0;

                    if(split_files && file_size >= 0xFFFF0001LL) {
                        use_split = 1;
                        sprintf(string, "%s.666%2.2u", path2, 0);
                        fp2 = fopen(string, "wb");

                    } else
                        fp2 = fopen(path2, "wb");

                    if(fp2) {
                        
                        fp_split0 = fp;
                        
                        u32 count = 0, percent = (u32) (file_size / 0x40000ULL);
                        if(percent == 0) percent = 1;

                        int count_split = 0;

                        clock_t t_one, t_two;

                        t_one = clock();

                        while(file_size > 0) {
                            u32 fsize;

                            t_two = clock();

                            if(((t_two - t_one) >= CLOCKS_PER_SEC/2)) {
                                t_one = t_two;
                                if(verbose) printf("\r*** Writing... %u %%", count * 100 / percent);
                            }

                             

                            if(use_split && count_split >= 0x40000000) {
                                count_split = 0;
                                fclose(fp2);
                                
                                sprintf(string, "%s.666%2.2u", path2, use_split);
                                use_split++;
                                fp2 = fopen(string, "wb");

                                if(!fp2) {
                                    printf("\nError!: creating extract file\n\n");
                                    goto err;
                                }

                            }

                            if(file_size > 0x40000) fsize = 0x40000;
                            else fsize = (u32) file_size;

                            count++;
                            
                            if(use_split) count_split+= fsize;

                            if(read_split(((u64) file_lba) * 2048ULL, (void *) sectors3, (int) fsize) < 0) {
                                printf("\nError!: reading ISO file\n\n");
                                goto err;
                            }

                            if(fwrite((void *) sectors3, 1, (int) fsize, fp2) != fsize) {
                                printf("\nError!: writing ISO file\n\n");
                                goto err;
                            }

                            file_size-= (u64) fsize;

                            file_lba += (fsize + 2047)/ 2048;
                            flba += (fsize + 2047)/ 2048;

                            if(!verbose) printf("\rPercent done: %u%% \r", (u32) (((u64) flba) * 100ULL / ((u64) toc)));
                        }

                        if(verbose) printf("\r                             \r");

                        fclose(fp2); fp2 = NULL;
                    } else {

                        printf("\nError!: creating extract file\n\n");
                        goto err;

                    }

                    path2[len_path2] = 0;
                    string2[len] = 0;

                   
                }

                q+= idr->length[0];
            }

            lba ++; 
            q2+= 2048;
            if(q2 >= size_directory) goto end_dir_rec;

        }

        end_dir_rec:

        p+= snamelen;
        if(snamelen & 1) p++;

        idx++;

    }

    if(!verbose) printf("\rPercent done: %u%% \n", 100);

    if(fp) fclose(fp);
    if(fp2) fclose(fp2);
    if(split_index && fp_split) {fclose(fp_split); fp_split = NULL;}
    if(sectors) free(sectors);
    if(sectors2) free(sectors2);
    if(sectors3) free(sectors3);

    for(n = 0; n <= idx; n++)
        if(directory_iso2[n].name) {free(directory_iso2[n].name); directory_iso2[n].name = NULL;}
    
    if(directory_iso2) free(directory_iso2);

    t_finish = clock();    

    if(verbose) printf("Finish!\n\n");
    if(verbose) printf("Total Time (HH:MM:SS): %2.2u:%2.2u:%2.2u.%u\n\n", (u32) ((t_finish - t_start)/(CLOCKS_PER_SEC * 3600)),
        (u32) (((t_finish - t_start)/(CLOCKS_PER_SEC * 60)) % 60), (u32) (((t_finish - t_start)/(CLOCKS_PER_SEC)) % 60),
        (u32) (((t_finish - t_start)/(CLOCKS_PER_SEC/100)) % 100));

    if(argc < 2) {
        printf("\nPress ENTER key to exit\n");
        get_input_char();
    }

    return 0;

err:

    if(fp) fclose(fp);
    if(fp2) fclose(fp2);
    if(split_index && fp_split) {fclose(fp_split); fp_split = NULL;}

    if(sectors) free(sectors);
    if(sectors2) free(sectors2);
    if(sectors3) free(sectors3);

    for(n = 0; n <= idx; n++)
        if(directory_iso2[n].name) {free(directory_iso2[n].name); directory_iso2[n].name = NULL;}
    
    if(directory_iso2) free(directory_iso2);

    printf("\nPress ENTER key to exit\n");
    get_input_char();

    return -1;
}
