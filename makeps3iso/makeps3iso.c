/* 
    (c) 2013 Estwald/Hermes <www.elotrolado.net>

    MAKEPS3ISO is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MAKEPS3ISO is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    apayloadlong with MAKEPS3ISO.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <ctype.h>

int verbose = 1;

//#define ALIGNED32SECTORS 1
#define NOPS3_UPDATE 1

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
#include <wincrypt.h>

// get_rand() method from ps3netsrv Cobra Sources...

static void get_rand(void *bfr, u32 size)
{
	HCRYPTPROV hProv;
	
	if (size == 0)
		return;

	if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
        {fprintf(stderr, "Error aquiring crypt context.\n"); return;}
	
	if (!CryptGenRandom(hProv, size, (BYTE *)bfr))
		fprintf(stderr, "Errorgetting random numbers.\n");

	CryptReleaseContext(hProv, 0);
}

static u64 get_disk_free_space(char *path)
{
 
    DWORD SectorsPerCluster, BytesPerSector, NumberOfFreeClusters, TotalNumberOfClusters;
    
    if(!GetDiskFreeSpace(path, &SectorsPerCluster, &BytesPerSector, &NumberOfFreeClusters, &TotalNumberOfClusters))
        return (u64) (-1LL);

    return ((u64) BytesPerSector) * ((u64) SectorsPerCluster) * ((u64) NumberOfFreeClusters);

}
#elif __unix__ 
#include <sys/statvfs.h>

// get_rand() method from ps3netsrv Cobra Sources...

static void get_rand(void *bfr, u32 size)
{
	FILE *fp;
	
	if (size == 0)
		return;

	fp = fopen("/dev/urandom", "r");
	if (fp == NULL) {
		fprintf(stderr, "Error aquiring crypt context.\n");
        return;
    }

	if (fread(bfr, size, 1, fp) != 1)
		fprintf(stderr, "Error getting random numbers.\n");

	fclose(fp);
}

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

static void setdaterec(unsigned char *p,int dd,int mm,int aa,int ho,int mi,int se)
{
    *p++=(unsigned char) ((aa-1900) & 255);*p++=(char) (mm & 15) ;*p++=(char) (dd & 31);*p++=(char) ho;*p++=(char) mi;*p++=(char) se;*p++=(char) 0;

}

static void set731(unsigned char *p,int n)
{
    *p++=(n & 0xff);*p++=((n>>8) & 0xff);*p++=((n>>16) & 0xff);*p++=((n>>24) & 0xff);
}

static void set733(unsigned char *p,int n)
{
    *p++=(n & 0xff);*p++=((n>>8) & 0xff);*p++=((n>>16) & 0xff);*p++=((n>>24) & 0xff);
    *p++=((n>>24) & 0xff);*p++=((n>>16) & 0xff);*p++=((n>>8) & 0xff);*p++=(n & 0xff);
}

static void set732(unsigned char *p,int n)
{
    *p++=((n>>24) & 0xff);*p++=((n>>16) & 0xff);*p++=((n>>8) & 0xff);*p++=(n & 0xff);
}

static void set721(unsigned char *p,int n)
{
    *p++=(n & 0xff);*p++=((n>>8) & 0xff);
}

static void set722(unsigned char *p,int n)
{
    *p++=((n>>8) & 0xff);*p++=(n & 0xff);
}

static void set723(unsigned char *p,int n)
{
    *p++=(n & 0xff);*p++=((n>>8) & 0xff);*p++=((n>>8) & 0xff);*p++=(n & 0xff);
}

#if 0

static int isonum_731 (unsigned char * p)
{
	return ((p[0] & 0xff)
		| ((p[1] & 0xff) << 8)
		| ((p[2] & 0xff) << 16)
		| ((p[3] & 0xff) << 24));
}


static int isonum_732 (unsigned char * p)
{
	return ((p[3] & 0xff)
		| ((p[2] & 0xff) << 8)
		| ((p[1] & 0xff) << 16)
		| ((p[0] & 0xff) << 24));
}


static int isonum_733 (unsigned char * p)
{
	return (isonum_731 (p));
}


static int isonum_721 (char * p)
{
	return ((p[0] & 0xff) | ((p[1] & 0xff) << 8));
}

static int isonum_723 (char * p)
{

	return (isonum_721 (p));
}

#endif

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

static void UTF8_to_UTF16(u8 *stb, u16 *stw)
{
   int n, m;
   u32 UTF32;
   while(*stb) {
       if(*stb & 128) {
            m = 1;

            if((*stb & 0xf8)==0xf0) { // 4 bytes
                UTF32 = (u32) (*(stb++) & 3);
                m = 3;
            } else if((*stb & 0xE0)==0xE0) { // 3 bytes
                UTF32 = (u32) (*(stb++) & 0xf);
                m = 2;
            } else if((*stb & 0xE0)==0xC0) { // 2 bytes
                UTF32 = (u32) (*(stb++) & 0x1f);
                m = 1;
            } else {stb++;continue;} // error!

             for(n = 0; n < m; n++) {
                if(!*stb) break; // error!
                    if((*stb & 0xc0) != 0x80) break; // error!
                    UTF32 = (UTF32 <<6) |((u32) (*(stb++) & 63));
             }
           
            if((n != m) && !*stb) break;
        
        } else UTF32 = (u32) *(stb++);

        if(UTF32<65536)
            *stw++= SWAP16((u16) UTF32);
        else {//110110ww wwzzzzyy 110111yy yyxxxxxx
            *stw++= SWAP16((((u16) (UTF32>>10)) & 0x3ff) | 0xD800);
            *stw++= SWAP16((((u16) (UTF32)) & 0x3ff) | 0xDC00);
        }
   }

   *stw++ = 0;
}

static void utf8_to_ansiname(char *utf8, char *ansi, int len)
{
u8 *ch= (u8 *) utf8;
u8 c;
int is_space = 1;

char *a = ansi;

    *ansi = 0;

	while(*ch!=0 && len>0){

	// 3, 4 bytes utf-8 code 
	if(((*ch & 0xF1)==0xF0 || (*ch & 0xF0)==0xe0) && (*(ch+1) & 0xc0) == 0x80){

	if(!is_space) {
        *ansi++=' '; // ignore
        len--;
        is_space = 1;
    }
	
	ch+= 2+1*((*ch & 0xF1) == 0xF0);
	
	}
	else 
	// 2 bytes utf-8 code	
	if((*ch & 0xE0)==0xc0 && (*(ch+1) & 0xc0)==0x80){
	
        c= (((*ch & 3)<<6) | (*(ch+1) & 63));

        if(c>=0xC0 && c<=0xC5) c='A';
        else if(c==0xc7) c='C';
        else if(c>=0xc8 && c<=0xcb) c='E';
        else if(c>=0xcc && c<=0xcf) c='I';
        else if(c==0xd1) c='N';
        else if(c>=0xd2 && c<=0xd6) c='O';
        else if(c>=0xd9 && c<=0xdc) c='U';
        else if(c==0xdd) c='Y';
        else if(c>=0xe0 && c<=0xe5) c='a';
        else if(c==0xe7) c='c';
        else if(c>=0xe8 && c<=0xeb) c='e';
        else if(c>=0xec && c<=0xef) c='i';
        else if(c==0xf1) c='n';
        else if(c>=0xf2 && c<=0xf6) c='o';
        else if(c>=0xf9 && c<=0xfc) c='u';
        else if(c==0xfd || c==0xff) c='y';
        else if(c>127) c=*(++ch+1); //' ';

        if(!is_space || c!= 32) {
           *ansi++=c;
            len--;
            if(c == 32) is_space = 1; else is_space = 0;
        }

	    ch++;
	
	} else {
	
        if(*ch<32) *ch=32;

        if(!is_space || *ch!= 32) {
           *ansi++=*ch;
        
            len--;

            if(*ch == 32) is_space = 1; else is_space = 0;
        }
	
	}

	ch++;
	}
	
    while(len>0) {
	    *ansi++=0;
	    len--;
	}

    if(a[0] == 0 || a[0] == ' ') strcpy(a, "PS3");

}

static int cur_isop = -1;

static int lpath;
static int wpath;

static u32 llba0 = 0; // directory path0
static u32 llba1 = 0; // directory path1
static u32 wlba0 = 0; // directory pathw0
static u32 wlba1 = 0; // directory pathw1

static u32 dllba = 0; // dir entries
static u32 dwlba = 0; // dir entriesw
static u32 dlsz = 0; // dir entries size (sectors)
static u32 dwsz = 0; // dir entriesw size (sectors)
static u32 flba = 0; // first lba for files
static u32 toc = 0;  // TOC of the iso

static char iso_split = 0;
static char output_name[0x420];

static int pos_lpath0 = 0;
static int pos_lpath1 = 0;
static int pos_wpath0 = 0;
static int pos_wpath1 = 0;

static int dd = 1, mm = 1, aa = 2013, ho = 0, mi = 0, se = 2;

static u8 *sectors = NULL;


#define MAX_ISO_PATHS 4096

typedef struct {
    u32 ldir;
    u32 wdir;
    u32 llba;
    u32 wlba;
    int parent;
    char *name;

} _directory_iso;

static _directory_iso *directory_iso = NULL;

static u16 wstring[1024];

static char temp_string[1024];

static void memcapscpy(void *dest, void *src, int size)
{
    char *d = dest;
    char *s = src;
    char c;

    int n;

    for(n = 0; n < size; n++) {c = *s++; *d++ =toupper(c);}
}

static int parse_param_sfo(char * file, char *title_id, char *title_name)
{
	FILE *fp;
    int bytes;
    int ct = 0;
    
    fp = fopen(file, "rb");

	if(fp)
		{
		int len, pos, str;
		unsigned char *mem=NULL;

        fseek(fp, 0, SEEK_END);
		len =  ftell(fp);

		mem= (unsigned char *) malloc(len+16);
		if(!mem) {fclose(fp);return -2;}

		memset(mem, 0, len+16);

		fseek(fp, 0, SEEK_SET);

        bytes = fread((void *) mem, 1, len, fp);

        fclose(fp);

        if(bytes != len) {
            free(mem);
            return -2;
        }

		str= (mem[8]+(mem[9]<<8));
		pos=(mem[0xc]+(mem[0xd]<<8));

		int indx=0;
        
		while(str<len) {
			if(mem[str]==0) break;

            if(!strcmp((char *) &mem[str], "TITLE")) {
                strncpy(title_name, (char *) &mem[pos], 63);
                ct++;
            }
            else 
			if(!strcmp((char *) &mem[str], "TITLE_ID")) {
                memcpy(title_id, (char *) &mem[pos], 4);
                title_id[4] = '-';
				strncpy(&title_id[5], (char *) &mem[pos + 4], 58);
                ct++;
				
		    }

            if(ct == 2) {
                free(mem);
				return 0;
            }

			while(mem[str]) str++;str++;
			pos+=(mem[0x1c+indx]+(mem[0x1d+indx]<<8));
			indx+=16;
		}

		if(mem) free(mem);
        
		}

	
	return -1;

}

int print_cycle = 0;

char cycle_str[4][2] = {"/", "-", "\\", "|" };


static int calc_entries(char *path, int parent)
{
    DIR  *dir;
    int len_string;
    struct stat s;

    int cldir = 0;
    int ldir = sizeof(struct iso_directory_record) + 6; // ..
    ldir = (ldir + 7) & ~7;
    ldir += sizeof(struct iso_directory_record) + 6; // .
    ldir += ldir & 1;
    
    int cwdir = 0;
    int wdir = sizeof(struct iso_directory_record) + 6; // ..
    wdir = (wdir + 7) & ~7;
    wdir += sizeof(struct iso_directory_record) + 6; // .
    wdir += wdir & 1;

    cldir = ldir;
    cwdir = wdir;

    lpath+= (lpath & 1);
    wpath+= (wpath & 1);

    int cur = cur_isop;

    if(cur >= MAX_ISO_PATHS) return -444;

    directory_iso[cur].ldir = ldir;
    directory_iso[cur].wdir = wdir;
    directory_iso[cur].parent = parent;
    if(!cur) {
        directory_iso[cur].name = malloc(16); 
        if(!directory_iso[cur].name) return -1;
        directory_iso[cur].name[0] = 0;
    }


    int cur2 = cur;

    cur_isop++;

    // files
    dir = opendir (path);
    if(dir) {
        while(1) {
        
        struct dirent *entry = readdir (dir);
            
        if(!entry) break;
        if(entry->d_name[0]=='.' && (entry->d_name[1]=='.' || entry->d_name[1]== 0)) continue;

        int len = strlen(path);
        strcat(path,"/");
        strcat(path, entry->d_name);

        if(stat(path, &s)<0) {closedir(dir); return -669;}
        
        path[len] = 0;

        if(verbose) printf("\r%s    \r", &cycle_str[print_cycle][0]); print_cycle++; print_cycle&= 3;

        if(!S_ISDIR(s.st_mode)) {
            
            int lname = strlen(entry->d_name);

            if(lname >=6 && !strcmp(&entry->d_name[lname -6], ".66600")) { // build size of .666xx files
                u64 size = s.st_size;
                lname -= 6;
                int n;

                memcpy(temp_string, entry->d_name, lname);
                temp_string[lname] = 0;

                if(lname > 222) {closedir(dir); return -555;}

                UTF8_to_UTF16((u8 *) temp_string, wstring);

                for(len_string = 0; len_string < 512; len_string++) if(wstring[len_string] == 0) break;

                if(len_string > 222) {closedir(dir); return -555;}


                path[len] = 0;

                for(n = 1; n < 100; n++) {

                    int len2 = strlen(path);
                    strcat(path,"/");

                    int l = strlen(path);

                    memcpy(path + l, entry->d_name, lname);
                    

                    sprintf(&path[l + lname], ".666%2.2u", n);

                    if(stat(path, &s)<0) {s.st_size = size; path[len2] = 0; break;}
                    
                    path[len2] = 0;
                    
                    size += s.st_size;     
                    
                }

                path[len] = 0;

            
            } else {
                if(lname >=6 && !strncmp(&entry->d_name[lname -6], ".666", 4)) continue; // ignore .666xx files
                else {
                
                    if(strlen(entry->d_name) > 222) {closedir(dir); return -555;}

                    UTF8_to_UTF16((u8 *) entry->d_name, wstring);

                    for(len_string = 0; len_string < 512; len_string++) if(wstring[len_string] == 0) break;

                    if(len_string > 222) {closedir(dir); return -555;}
                }
            }


            int parts = s.st_size ? (int) ((((u64) s.st_size) + 0xFFFFF7FFULL)/0xFFFFF800ULL) : 1;

            int n;

            for(n = 0; n < parts; n++) {

                int add;

                add = sizeof(struct iso_directory_record) + lname - 1 + 8; // add ";1"
                add+= add & 1;
                cldir += add;

                if(cldir > 2048) {
                
                    ldir = (ldir & ~2047) + 2048;
                    cldir = add;
                } else if(cldir == 2048){
                    cldir = 0;
                }

                ldir += add;
                //ldir += ldir & 1;

                add = sizeof(struct iso_directory_record) - 1 + len_string * 2  + 4 + 6;  // add "\0;\01"
                add+= add & 1;
                
                cwdir += add;

                if(cwdir > 2048) {
                 
                    wdir= (wdir & ~2047) + 2048;
                    cwdir = add;
                } else if(cwdir == 2048){
                    cwdir = 0;
                }

                wdir += add;
                //wdir += wdir & 1;
            }
        }

    }

    closedir (dir);
    
    // directories
    dir = opendir (path);
    if(dir) {
        while(1) {
        
            struct dirent *entry = readdir (dir);
                
            if(!entry) break;
            if(entry->d_name[0]=='.' && (entry->d_name[1]=='.' || entry->d_name[1]== 0)) continue;

            int len = strlen(path);
            strcat(path,"/");
            strcat(path, entry->d_name);

            if(stat(path, &s)<0) {closedir(dir); return -669;}
            
            path[len] = 0;

            if(verbose) printf("\r%s    \r", &cycle_str[print_cycle][0]); print_cycle++; print_cycle&= 3;

            if(S_ISDIR(s.st_mode)) {

                if(strlen(entry->d_name) > 222) {closedir(dir); return -555;}

                UTF8_to_UTF16((u8 *) entry->d_name, wstring);

                for(len_string = 0; len_string < 512; len_string++) if(wstring[len_string] == 0) break;

                if(len_string > 222) {closedir(dir); return -555;}
                

                lpath += sizeof(struct iso_path_table) + strlen(entry->d_name) - 1;
                lpath += (lpath & 1);

                int add;

                add = sizeof(struct iso_directory_record) + strlen(entry->d_name) - 1 + 6;
                add+= add & 1;
                cldir += add;

                if(cldir > 2048) {
                    
                    ldir = (ldir & ~2047) + 2048;
                    cldir = add;
                } else if(cldir == 2048){
                    cldir = 0;
                }
                
                ldir += add;
                //ldir += ldir & 1;

                wpath += sizeof(struct iso_path_table) + len_string * 2 - 1;
                wpath += (wpath & 1);

                add = sizeof(struct iso_directory_record) + len_string * 2 - 1 + 6;
                add+= add & 1;
                cwdir += add;

                if(cwdir > 2048) {

                    wdir= (wdir & ~2047) + 2048;
                    cwdir = add;
                } else if(cwdir == 2048){
                    cwdir = 0;
                }

                wdir += add;
                //wdir += wdir & 1;
        
            }
        }

        closedir (dir);
    }

    //if((2048 - (ldir & 2047)) < 256/*sizeof(struct iso_directory_record)*/) ldir += 2048; // increase an sector if not space for directory record
    //if((2048 - (wdir & 2047)) < 256/*sizeof(struct iso_directory_record)*/) wdir += 2048; // increase an sector if not space for directory record

    directory_iso[cur].ldir = (ldir + 2047)/2048;
    directory_iso[cur].wdir = (wdir + 2047)/2048;

    }

    // directories add
    dir = opendir (path);
    if(dir) {
        while(1) {
        
        struct dirent *entry = readdir (dir);
            
        if(!entry) break;
        if(entry->d_name[0]=='.' && (entry->d_name[1]=='.' || entry->d_name[1]== 0)) continue;
        
        int len = strlen(path);

        strcat(path,"/");
        strcat(path, entry->d_name);

        if(stat(path, &s)<0) {closedir(dir); return -669;}
           

        if(!(S_ISDIR(s.st_mode))) {path[len] = 0; continue;}

        //printf("ss %s\n", path);

        directory_iso[cur_isop].name = malloc(strlen(entry->d_name) + 1);
        if(!directory_iso[cur_isop].name) {closedir(dir); return -1;}
        strcpy(directory_iso[cur_isop].name, entry->d_name);
        
        int ret = calc_entries(path, cur2 + 1);

        if(ret < 0) {closedir(dir); return ret;}

        path[len] = 0;

        }
        closedir (dir);
    }

    if(cur == 0) {

        llba0 = 20;
        llba1 = llba0 + ((lpath + 2047)/2048);
        wlba0 = llba1 + ((lpath + 2047)/2048);
        wlba1 = wlba0 + ((wpath + 2047)/2048);
        dllba = wlba1 + ((wpath + 2047)/2048);
        if(dllba < 32) dllba = 32;

        int n;
        
        int m, l;

        // searching...

        for(n = 1; n < cur_isop - 1; n++)
            for(m = n + 1; m < cur_isop; m++) {
            
                if(directory_iso[n].parent > directory_iso[m].parent) {
             
                    directory_iso[cur_isop] = directory_iso[n]; directory_iso[n] = directory_iso[m]; directory_iso[m] = directory_iso[cur_isop];
                  
                    for(l = n; l < cur_isop; l++) {
                 
                        if(n + 1 == directory_iso[l].parent) 
                            directory_iso[l].parent = m + 1;
                        else if(m + 1 == directory_iso[l].parent) 
                            directory_iso[l].parent = n + 1;
                    }

                }
        }

        /*for(n = 0; n < cur_isop; n++) {

            printf("list %i %s\n", directory_iso[n].parent, directory_iso[n].name);
        }*/
        
        for(n = 0; n < cur_isop; n++)  {
            dlsz+= directory_iso[n].ldir;
            dwsz+= directory_iso[n].wdir;
        }

        #ifdef ALIGNED32SECTORS
        dwlba = ((dllba + dlsz) + 31) & ~31;

        flba = ((dwlba + dwsz) + 31) & ~31;
        #else
        dwlba = (dllba + dlsz);
        flba = (dwlba + dwsz);
        #endif

        u32 lba0 = dllba;
        u32 lba1 = dwlba;

        for(n = 0; n < cur_isop; n++)  {
            
            directory_iso[n].llba = lba0;
            directory_iso[n].wlba = lba1;
            lba0 += directory_iso[n].ldir;
            lba1 += directory_iso[n].wdir;
            
        }

        if(verbose) printf("\r        \r");;
    }


/*
if(cur == 0) {
    int n;
        for(n = 0; n < cur_isop; n++)  {
            printf("list %i %s\n", directory_iso[n].parent, directory_iso[n].name);
        }
}
*/

    return 0;

}

static int fill_dirpath(void)
{
    int n;
    struct iso_path_table *iptl0;
    struct iso_path_table *iptl1;
    struct iso_path_table *iptw0;
    struct iso_path_table *iptw1;

    for(n = 0; n < cur_isop; n++) {

        iptl0 = (void *) &sectors[pos_lpath0];
        iptl1 = (void *) &sectors[pos_lpath1];
        iptw0 = (void *) &sectors[pos_wpath0];
        iptw1 = (void *) &sectors[pos_wpath1];

        if(!n) {
            set721((void *) iptl0->name_len, 1);
            set731((void *) iptl0->extent, directory_iso[n].llba);
            set721((void *) iptl0->parent, directory_iso[n].parent);
            iptl0->name[0] = 0;
            pos_lpath0 += sizeof(struct iso_path_table) - 1 + 1;
            pos_lpath0 += pos_lpath0 & 1;
            iptl0 = (void *) &sectors[pos_lpath0];

            set721((void *) iptl1->name_len, 1);
            set732((void *) iptl1->extent, directory_iso[n].llba);
            set722((void *) iptl1->parent, directory_iso[n].parent);
            iptl1->name[0] = 0;
            pos_lpath1 += sizeof(struct iso_path_table) - 1 + 1;
            pos_lpath1 += pos_lpath1 & 1;
            iptl1 = (void *) &sectors[pos_lpath1];

            set721((void *) iptw0->name_len, 1);
            set731((void *) iptw0->extent, directory_iso[n].wlba);
            set721((void *) iptw0->parent, directory_iso[n].parent);
            iptw0->name[0] = 0;
            pos_wpath0 += sizeof(struct iso_path_table) - 1 + 1;
            pos_wpath0 += pos_wpath0 & 1;
            iptw0 = (void *) &sectors[pos_wpath0];

            set721((void *) iptw1->name_len, 1);
            set732((void *) iptw1->extent, directory_iso[n].wlba);
            set722((void *) iptw1->parent, directory_iso[n].parent);
            iptw1->name[0] = 0;
            pos_wpath1 += sizeof(struct iso_path_table) - 1 + 1;
            pos_wpath1 += pos_wpath1 & 1;
            iptw1 = (void *) &sectors[pos_wpath1];
            continue;
            
        }

        //////
        UTF8_to_UTF16((u8 *) directory_iso[n].name, wstring);

        int len_string;

        for(len_string = 0; len_string < 512; len_string++) if(wstring[len_string] == 0) break;


        set721((void *) iptl0->name_len, strlen(directory_iso[n].name));
        set731((void *) iptl0->extent, directory_iso[n].llba);
        set721((void *) iptl0->parent, directory_iso[n].parent);
        memcapscpy(&iptl0->name[0], directory_iso[n].name, strlen(directory_iso[n].name));
        pos_lpath0 += sizeof(struct iso_path_table) - 1 + strlen(directory_iso[n].name);
        pos_lpath0 += pos_lpath0 & 1;
        iptl0 = (void *) &sectors[pos_lpath0];

        set721((void *) iptl1->name_len, strlen(directory_iso[n].name));
        set732((void *) iptl1->extent, directory_iso[n].llba);
        set722((void *) iptl1->parent, directory_iso[n].parent);
        memcapscpy(&iptl1->name[0], directory_iso[n].name, strlen(directory_iso[n].name));
        pos_lpath1 += sizeof(struct iso_path_table) - 1 + strlen(directory_iso[n].name);
        pos_lpath1 += pos_lpath1 & 1;
        iptl1 = (void *) &sectors[pos_lpath1];

        set721((void *) iptw0->name_len, len_string * 2);
        set731((void *) iptw0->extent, directory_iso[n].wlba);
        set721((void *) iptw0->parent, directory_iso[n].parent);
        memcpy(&iptw0->name[0], wstring, len_string * 2);
        pos_wpath0 += sizeof(struct iso_path_table) - 1 + len_string * 2;
        pos_wpath0 += pos_wpath0 & 1;
        iptw0 = (void *) &sectors[pos_wpath0];

        set721((void *) iptw1->name_len, len_string * 2);
        set732((void *) iptw1->extent, directory_iso[n].wlba);
        set722((void *) iptw1->parent, directory_iso[n].parent);
        memcpy(&iptw1->name[0], wstring, len_string * 2);
        pos_wpath1 += sizeof(struct iso_path_table) - 1 + len_string * 2;
        pos_wpath1 += pos_wpath1 & 1;
        iptw1 = (void *) &sectors[pos_wpath1];     

        //////

    }
    
    return 0;
}

static int fill_entries(char *path1, char *path2, int level)
{
    DIR  *dir;

    int n;
    int len_string;

    int len1 = strlen(path1);
    int len2 = strlen(path2);

    struct iso_directory_record *idrl = (void *) &sectors[directory_iso[level].llba * 2048];
    struct iso_directory_record *idrw = (void *) &sectors[directory_iso[level].wlba * 2048];
    struct iso_directory_record *idrl0 = idrl;
    struct iso_directory_record *idrw0 = idrw;

    struct tm *tm;
    struct stat s;

    memset((void *) idrl, 0, 2048);
    memset((void *) idrw, 0, 2048);

    u32 count_sec1 = 1, count_sec2 = 1, max_sec1, max_sec2;

    int first_file = 1;
    
    int aux_parent = directory_iso[level].parent - 1;

    if(level!=0) {
        strcat(path2, "/");
        strcat(path2, directory_iso[level].name);
        strcat(path1, path2);
    } else {path2[0] = 0; fill_dirpath();}


    //printf("q %s LBA %X\n", path2, directory_iso[level].llba);

    if(stat(path1, &s)<0) {return -669;}

    tm = localtime(&s.st_mtime);
    dd = tm->tm_mday;
    mm = tm->tm_mon + 1;
    aa = tm->tm_year + 1900;
    ho = tm->tm_hour;
    mi = tm->tm_min;
    se = tm->tm_sec;

    idrl->length[0] = sizeof(struct iso_directory_record) + 6;
    idrl->length[0] += idrl->length[0] & 1;
    idrl->ext_attr_length[0] = 0;
    set733((void *) idrl->extent, directory_iso[level].llba);
    set733((void *) idrl->size, directory_iso[level].ldir * 2048);
    setdaterec(idrl->date, dd, mm, aa, ho, mi, se);
    idrl->flags[0] = 0x2;
    idrl->file_unit_size[0] = 0x0;
    idrl->interleave[0] = 0x0;
    set723((void *) idrl->volume_sequence_number, 1);
    idrl->name_len[0] = 1;
    idrl->name[0] = 0;
    idrl = (void *) ((char *) idrl) + idrl->length[0];

    max_sec1 = directory_iso[level].ldir;

    idrw->length[0] = sizeof(struct iso_directory_record) + 6;
    idrw->length[0] += idrw->length[0] & 1;
    idrw->ext_attr_length[0] = 0;
    set733((void *) idrw->extent, directory_iso[level].wlba);
    set733((void *) idrw->size, directory_iso[level].wdir * 2048);
    setdaterec(idrw->date, dd, mm, aa, ho, mi, se);
    idrw->flags[0] = 0x2;
    idrw->file_unit_size[0] = 0x0;
    idrw->interleave[0] = 0x0;
    set723((void *) idrw->volume_sequence_number, 1);
    idrw->name_len[0] = 1;
    idrw->name[0] = 0;
    idrw = (void *) ((char *) idrw) + idrw->length[0];

    max_sec2 = directory_iso[level].wdir;

    if(level) {
        int len = strlen(path1);
        strcat(path1,"/..");
        if(stat(path1, &s)<0) {return -669;}
        path1[len] = 0;

        tm = localtime(&s.st_mtime);
        dd = tm->tm_mday;
        mm = tm->tm_mon + 1;
        aa = tm->tm_year + 1900;
        ho = tm->tm_hour;
        mi = tm->tm_min;
        se = tm->tm_sec;
    }

    idrl->length[0] = sizeof(struct iso_directory_record) + 6;
    idrl->length[0] += idrl->length[0] & 1;
    idrl->ext_attr_length[0] = 0;
    set733((void *) idrl->extent, directory_iso[!level ? 0 : aux_parent].llba);
    set733((void *) idrl->size, directory_iso[!level ? 0 : aux_parent].ldir * 2048);
    setdaterec(idrl->date, dd, mm, aa, ho, mi, se);
    idrl->flags[0] = 0x2;
    idrl->file_unit_size[0] = 0x0;
    idrl->interleave[0] = 0x0;
    set723((void *) idrl->volume_sequence_number, 1);
    idrl->name_len[0] = 1;
    idrl->name[0] = 1;
    idrl = (void *) ((char *) idrl) + idrl->length[0];

    idrw->length[0] = sizeof(struct iso_directory_record) + 6;
    idrw->length[0] += idrw->length[0] & 1;
    idrw->ext_attr_length[0] = 0;
    set733((void *) idrw->extent, directory_iso[!level ? 0 : aux_parent].wlba);
    set733((void *) idrw->size, directory_iso[!level ? 0 : aux_parent].wdir * 2048);
    setdaterec(idrw->date, dd, mm, aa, ho, mi, se);
    idrw->flags[0] = 0x2;
    idrw->file_unit_size[0] = 0x0;
    idrw->interleave[0] = 0x0;
    set723((void *) idrw->volume_sequence_number, 1);
    idrw->name_len[0] = 1;
    idrw->name[0] = 1;
    idrw = (void *) ((char *) idrw) + idrw->length[0];

    // files
    dir = opendir (path1);
    if(dir) {
        while(1) {
        
            struct dirent *entry = readdir (dir);
                
            if(!entry) break;
            if(entry->d_name[0]=='.' && (entry->d_name[1]=='.' || entry->d_name[1]== 0)) continue;
            
            int len = strlen(path1);

            #ifdef NOPS3_UPDATE
            if(!strcmp(&path1[len - 10], "PS3_UPDATE")) continue;
            #endif

            strcat(path1,"/");
            strcat(path1, entry->d_name);

            if(stat(path1, &s)<0) {closedir(dir); return -669;}

            path1[len] = 0;

            if(S_ISDIR(s.st_mode)) continue;

            //printf("\r2 %s    \r", &cycle_str[print_cycle][0]); print_cycle++; print_cycle&= 3;

            int lname = strlen(entry->d_name);

            if(lname >=6 && !strcmp(&entry->d_name[lname -6], ".66600")) { // build size of .666xx files
                u64 size = s.st_size;
                lname -= 6;
                int n;

                memcpy(temp_string, entry->d_name, lname);
                temp_string[lname] = 0;

                if(lname > 222) {closedir(dir); return -555;}

                UTF8_to_UTF16((u8 *) temp_string, wstring);

                for(len_string = 0; len_string < 512; len_string++) if(wstring[len_string] == 0) break;

                if(len_string > 222) {closedir(dir); return -555;}

                path1[len] = 0;

                for(n = 1; n < 100; n++) {

                    int len2 = strlen(path1);
                    strcat(path1,"/");

                    int l = strlen(path1);

                    memcpy(path1 + l, entry->d_name, lname);
                    
                    sprintf(&path1[l + lname], ".666%2.2u", n);

                    if(stat(path1, &s)<0) {s.st_size = size; path1[len2] = 0; break;}
                    
                    path1[len2] = 0;
                    
                    size += s.st_size;     
                    
                }

                path1[len] = 0;

            
            } else
                if(lname >=6 && !strncmp(&entry->d_name[lname -6], ".666", 4)) continue; // ignore .666xx files
                else {
                
                    if(strlen(entry->d_name) > 222) {closedir(dir); return -555;}

                    UTF8_to_UTF16((u8 *) entry->d_name, wstring);

                    for(len_string = 0; len_string < 512; len_string++) if(wstring[len_string] == 0) break;

                    if(len_string > 222) {closedir(dir); return -555;}
                }


            #ifdef ALIGNED32SECTORS
            if(first_file) flba= (flba + 31) & ~31;
            #endif

            first_file = 0;

            int parts = s.st_size ? (u32) ((((u64) s.st_size) + 0xFFFFF7FFULL)/0xFFFFF800ULL) : 1;

            int n;

            tm = gmtime(&s.st_mtime);
            time_t t = mktime(tm);
            tm = localtime(&t);
            dd = tm->tm_mday;
            mm = tm->tm_mon + 1;
            aa = tm->tm_year + 1900;
            ho = tm->tm_hour;
            mi = tm->tm_min;
            se = tm->tm_sec;


            for(n = 0; n < parts; n++) {

                u32 fsize;
                if(parts > 1 && (n + 1) != parts) {fsize = 0xFFFFF800; s.st_size-= fsize;}
                else fsize = s.st_size;

                int add;

                add = sizeof(struct iso_directory_record) - 1 + (lname + 8);
                add+= (add & 1);

                // align entry data with sector

                int cldir = (((int) idrl) - ((int) idrl0)) & 2047;
                
                cldir += add;

                if(cldir > 2048) {

                    //printf("gapl1 lba 0x%X %s/%s %i\n", directory_iso[level].llba, path1, entry->d_name, cldir);
                    //getchar();

                    count_sec1++;
                    if(count_sec1 > max_sec1) {
                        closedir (dir);
                        printf("Error!: too much entries in directory:\n%s\n", path1);
                        return -444;
                    }

                    idrl = (void *) ((char *) idrl) + (add - (cldir - 2048));

                    memset((void *) idrl, 0, 2048);
                    
                }

                idrl->length[0] = add;
                idrl->length[0] += idrl->length[0] & 1;
                idrl->ext_attr_length[0] = 0;
                set733((void *) idrl->extent, flba);
                set733((void *) idrl->size, fsize);
                
                //printf("a %i/%i/%i %i:%i:%i %s LBA: %X\n", dd, mm, aa, ho, mi, se, entry->d_name, flba);
                setdaterec((void *) idrl->date, dd, mm, aa, ho, mi, se);
                idrl->flags[0] = ((n + 1) != parts) ? 0x80 : 0x0; // fichero
                idrl->file_unit_size[0] = 0x0;
                idrl->interleave[0] = 0x0;
                set723((void *) idrl->volume_sequence_number, 1);
                idrl->name_len[0] = lname + 2;
                memcapscpy(idrl->name, entry->d_name, lname);
                idrl->name[lname + 0] = ';';
                idrl->name[lname + 1] = '1';

                idrl = (void *) ((char *) idrl) + idrl->length[0];
                
                //

                add = sizeof(struct iso_directory_record) - 1 + len_string * 2 + 4 + 6;
                add+= (add & 1);

                // align entry data with sector
                
                int cwdir = (((int) idrw) - ((int) idrw0)) & 2047;

                cwdir += add;

                if(cwdir > 2048) {

                    //printf("gapw1 lba 0x%X %s/%s %i\n", directory_iso[level].wlba, path1, entry->d_name, cwdir);
                    //getchar();
                    
                    count_sec2++;
                    if(count_sec2 > max_sec2) {
                        closedir (dir);
                        printf("Error!: too much entries in directory:\n%s\n", path1);
                        return -444;
                    }

                    idrw = (void *) ((char *) idrw) + (add - (cwdir - 2048));

                    memset((void *) idrw, 0, 2048);
                    
                }

                idrw->length[0] = add;
                idrw->length[0] += idrw->length[0] & 1;
                idrw->ext_attr_length[0] = 0;
                set733((void *) idrw->extent, flba);
                set733((void *) idrw->size, fsize);

                setdaterec((void *) idrw->date, dd, mm, aa, ho, mi, se);
                idrw->flags[0] = ((n + 1) != parts) ? 0x80 : 0x0; // fichero
                idrw->file_unit_size[0] = 0x0;
                idrw->interleave[0] = 0x0;
                set723((void *) idrw->volume_sequence_number, 1);
                idrw->name_len[0] = len_string * 2 + 4;
                memcpy(idrw->name, wstring, len_string * 2);
                idrw->name[len_string * 2 + 0] = 0;
                idrw->name[len_string * 2 + 1] = ';';
                idrw->name[len_string * 2 + 2] = 0;
                idrw->name[len_string * 2 + 3] = '1';
                idrw = (void *) ((char *) idrw) + idrw->length[0];

                flba+= ((fsize + 2047) & ~2047) / 2048;
            }

        }

    closedir (dir);
    }

   // printf("\r        \r");

    // folders
    for(n = 1; n < cur_isop; n++)
        if(directory_iso[n].parent == level + 1) {

            int len = strlen(path1);

            strcat(path1,"/");
            strcat(path1, directory_iso[n].name);

            if(stat(path1, &s)<0) {return -669;}

            path1[len] = 0;

            tm = localtime(&s.st_mtime);
            dd = tm->tm_mday;
            mm = tm->tm_mon + 1;
            aa = tm->tm_year + 1900;
            ho = tm->tm_hour;
            mi = tm->tm_min;
            se = tm->tm_sec;

            //printf("dir %i/%i/%i %i:%i:%i %s\n", dd, mm, aa, ho, mi, se, directory_iso[n].name);

            int add;

            add = sizeof(struct iso_directory_record) - 1 + (strlen(directory_iso[n].name) + 6);
            add+= (add & 1);

            // align entry data with sector
            
            int cldir = (((int)idrl) - ((int) idrl0)) & 2047;
            
            cldir += add;

            if(cldir > 2048) {

                //printf("gapl0 lba 0x%X %s/%s %i\n", directory_iso[level].llba, path1, directory_iso[n].name, cldir);
                //getchar();

                count_sec1++;
                if(count_sec1 > max_sec1) {
                    closedir (dir);
                    printf("Error!: too much entries in directory:\n%s\n", path1);
                    return -444;
                }

                idrl = (void *) ((char *) idrl) + (add - (cldir - 2048));

                memset((void *) idrl, 0, 2048);
                
            }

            idrl->length[0] = add;
            idrl->length[0] += idrl->length[0] & 1;
            idrl->ext_attr_length[0] = 0;
            set733((void *) idrl->extent, directory_iso[n].llba);
            set733((void *) idrl->size, directory_iso[n].ldir * 2048);
            setdaterec((void *) idrl->date, dd, mm, aa, ho, mi, se);
            idrl->flags[0] = 0x2;
            idrl->file_unit_size[0] = 0x0;
            idrl->interleave[0] = 0x0;
            set723((void *) idrl->volume_sequence_number, 1);
            idrl->name_len[0] = strlen(directory_iso[n].name);
            memcapscpy(idrl->name, directory_iso[n].name, strlen(directory_iso[n].name));
            idrl = (void *) ((char *) idrl) + idrl->length[0];

            //
            UTF8_to_UTF16((u8 *) directory_iso[n].name, wstring);

            for(len_string = 0; len_string < 512; len_string++) if(wstring[len_string] == 0) break;

            add = sizeof(struct iso_directory_record) - 1 + len_string * 2 + 6;
            add+= (add & 1);

            // align entry data with sector
            
            int cwdir = (((int) idrw) - ((int) idrw0)) & 2047;
            
            cwdir += add;

            if(cwdir > 2048) {

                //printf("gapw0 lba 0x%X %s/%s %i\n", directory_iso[level].wlba, path1, directory_iso[n].name, cwdir);
                //getchar();

                count_sec2++;
                if(count_sec2 > max_sec2) {
                    closedir (dir);
                    printf("Error!: too much entries in directory:\n%s\n", path1);
                    return -444;
                }

                idrw = (void *) ((char *) idrw) + (add - (cwdir - 2048));

                memset((void *) idrw, 0, 2048);
                
            }

            idrw->length[0] = add;
            idrw->length[0] += idrw->length[0] & 1;
            idrw->ext_attr_length[0] = 0;
            set733((void *) idrw->extent, directory_iso[n].wlba);
            set733((void *) idrw->size, directory_iso[n].wdir * 2048);
            setdaterec((void *) idrw->date, dd, mm, aa, ho, mi, se);
            idrw->flags[0] = 0x2;
            idrw->file_unit_size[0] = 0x0;
            idrw->interleave[0] = 0x0;
            set723((void *) idrw->volume_sequence_number, 1);
            idrw->name_len[0] = len_string * 2;
            memcpy(idrw->name, wstring, len_string * 2);
            idrw = (void *) ((char *) idrw) + idrw->length[0];

    }

    path1[len1] = 0;
    
    // iteration
    for(n = 1; n < cur_isop; n++)
        if(directory_iso[n].parent == level + 1) {
            int ret = fill_entries(path1, path2, n);
            if(ret < 0) {path2[len2] = 0; return ret;}
    }
    
    path2[len2] = 0;
   
    return 0;

}

#define SPLIT_LBA 0x1FFFE0

static int write_split(FILE *fp, u32 lba, u8 *mem, int sectors, int sel)
{
    char filename[0x420];

    if(!iso_split) {

        if(fwrite((void *) mem, 1, (int) sectors * 2048, fp) != sectors * 2048) return -667;
        return 0;
    }
    
    int cur = lba / SPLIT_LBA;
    int cur2 = (lba + sectors) / SPLIT_LBA;

    if(cur == cur2 && (iso_split - 1) == cur) {
        if(fwrite((void *) mem, 1, (int) sectors * 2048, fp) != sectors * 2048) return -667;
        return 0;
    }

    u32 lba2 = lba + sectors;
    u32 pos = 0;

    for(; lba < lba2; lba++) {

        int cur = lba / SPLIT_LBA;

        if(iso_split - 1 != cur) {
            if(fp) fclose(fp); fp = NULL;

            if(sel == 0) return 0;

            if(iso_split == 1) {
                sprintf(filename, "%s.0", output_name);
                remove(filename); //unlink
                rename(output_name, filename);
            }

            iso_split = cur + 1;

            sprintf(filename, "%s.%i", output_name, iso_split - 1);

            fp = fopen(filename, "wb");
            if(!fp) return -1;
            
        }
        
        if(fwrite((void *) mem + pos, 1, 2048 , fp) != 2048) return -667;
        pos += 2048;

    }

    return 0;
}

static int build_file_iso(FILE *fp, char *path1, char *path2, int level)
{
    DIR  *dir;
    struct stat s;

    int n;
    int first_file = 1;
    int len1 = strlen(path1);
    int len2 = strlen(path2);

    if(level!=0) {
        strcat(path2, "/");
        strcat(path2, directory_iso[level].name);
        strcat(path1, path2);
    } else path2[0] = 0;

    if(level)
        {if(verbose) printf("</%s>\n", directory_iso[level].name);}
    else
        {if(verbose) printf("</>\n");}

    //printf("q %s LBA %X\n", path2, directory_iso[level].llba);

    // files
    dir = opendir (path1);
    if(dir) {
        while(1) {
        
            struct dirent *entry=readdir (dir);
                
            if(!entry) break;
            if(entry->d_name[0]=='.' && (entry->d_name[1]=='.' || entry->d_name[1]== 0)) continue;
            
            int len = strlen(path1);

            #ifdef NOPS3_UPDATE
            if(!strcmp(&path1[len - 10], "PS3_UPDATE")) continue;
            #endif

            strcat(path1,"/");
            strcat(path1, entry->d_name);

            if(stat(path1, &s)<0) {path1[len] = 0;closedir(dir); return -669;}

            if(S_ISDIR(s.st_mode)) {path1[len] = 0; continue;}

            int is_file_split = 0;

            int lname = strlen(entry->d_name);

            if(lname >=6 && !strcmp(&entry->d_name[lname -6], ".66600")) { // build size of .666xx files
                u64 size = s.st_size;
                lname -= 6;
                int n;

                is_file_split = 1;

                memcpy(temp_string, entry->d_name, lname);
                temp_string[lname] = 0;

                if(lname > 222) {path1[len] = 0;closedir(dir); return -555;}

                path1[len] = 0;

                for(n = 1; n < 100; n++) {

                    int len2 = strlen(path1);
                    strcat(path1,"/");

                    int l = strlen(path1);

                    memcpy(path1 + l, entry->d_name, lname);
                    
                    sprintf(&path1[l + lname], ".666%2.2u", n);

                    if(stat(path1, &s)<0) {s.st_size = size; path1[len2] = 0; break;}
                    
                    path1[len2] = 0;
                    
                    size += s.st_size;     
                    
                }

                path1[len] = 0;
                strcat(path1,"/");
                strcat(path1, entry->d_name); // restore .66600 file

            
            } else
                if(lname >=6 && !strncmp(&entry->d_name[lname -6], ".666", 4)) {path1[len] = 0; continue;} // ignore .666xx files
                else {
                
                    if(strlen(entry->d_name) > 222) {path1[len] = 0; closedir(dir); return -555;}
   
                }

          
            FILE * fp2 = fopen(path1, "rb");
            path1[len] = 0;

            if(!fp2) {closedir(dir); return -666;}
            
            u32 flba0 = flba;

            #ifdef ALIGNED32SECTORS
            if(first_file) flba= (flba + 31) & ~31;
            #endif

            first_file = 0;

            if(flba0 != flba) {
                //printf("gap: %i\n", (flba - flba0));
                
                int f = (flba - flba0);
                int f2 = 0;
                int z = 0;
                
                memset(sectors, 0, ((f > 128) ? 128 : f ) * 2048);
                
                while(f > 0) { 
                    if(f > 128) f2 = 128; else f2 = f;
                  
                    int ret = write_split(fp, flba + z, sectors, f2, 1);
                    if(ret < 0) {
                        fclose(fp2); closedir(dir); return ret;

                    } 

                    f -= f2;
                    z += f2;
                }
            }
            
            if(is_file_split) {
                if(s.st_size < 1024ULL) 
                    {if(verbose) printf("  -> %s LBA %u size %u Bytes\n", temp_string, flba, (u32) s.st_size);}
                else if(s.st_size < 0x100000LL) 
                    {if(verbose) printf("  -> %s LBA %u size %u KB\n", temp_string, flba, (u32) (s.st_size/1024));}
                else
                    {if(verbose) printf("  -> %s LBA %u size %u MB\n", temp_string, flba, (u32) (s.st_size/0x100000LL));}
            } else {
                if(s.st_size < 1024ULL) 
                    {if(verbose) printf("  -> %s LBA %u size %u Bytes\n", entry->d_name, flba, (u32) s.st_size);}
                else if(s.st_size < 0x100000LL) 
                    {if(verbose) printf("  -> %s LBA %u size %u KB\n", entry->d_name, flba, (u32) (s.st_size/1024));}
                else
                    {if(verbose) printf("  -> %s LBA %u size %u MB\n", entry->d_name, flba, (u32) (s.st_size/0x100000LL));}
            }

            u32 count = 0, percent = (u32) (s.st_size / 0x40000ULL);
            if(percent == 0) percent = 1;

            clock_t t_one, t_two;

            t_one = clock();

            while(s.st_size > 0) {
                u32 fsize;
                u32 lsize;

                t_two = clock();

                if(((t_two - t_one) >= CLOCKS_PER_SEC/2)) {
                    t_one = t_two;
                    if(verbose) printf("\r*** Writing... %u %%", count * 100 / percent); 
                }

                if(s.st_size > 0x40000) fsize = 0x40000;
                else fsize = (u32) s.st_size;

                count++;
                
                if(fsize < 0x40000) memset(sectors, 0, 0x40000);

                if(is_file_split) {
                    int read = fread((void *) sectors, 1, (int) fsize, fp2);
                    if(read < 0) {
                        fclose(fp2); closedir(dir); return -668;
                    } else if(read < fsize) {
                        fclose(fp2);
                        path1[len] = 0;
                        strcat(path1,"/");

                        int l = strlen(path1);

                        memcpy(path1 + l, entry->d_name, lname);
                        
                        sprintf(&path1[l + lname], ".666%2.2u", is_file_split);
                        //printf("split: %s\n", path1);
                        is_file_split++;

                        fp2 = fopen(path1, "rb");
                        path1[len] = 0;

                        if(!fp2) {closedir(dir); return -666;}

                        if(fread((void *) (sectors + read), 1, (int) (fsize - read), fp2) != (fsize - read)) {
                            fclose(fp2); closedir(dir); return -668;
                        }
                        
                    } 

                } else {

                    if(fread((void *) sectors, 1, (int) fsize, fp2) != fsize) {
                        fclose(fp2); closedir(dir); return -668;
                    }
                }

                lsize = (fsize + 2047) & ~2047;

                int ret = write_split(fp, flba, sectors, lsize/2048, 1);

                if(ret < 0) {
                    if(verbose) printf("\n");
                    fclose(fp2); closedir(dir); return ret;

                }

                flba += lsize/2048;

                if(!verbose) printf("\rPercent done: %u%% \r", (u32) (((u64) flba) * 100ULL / ((u64) toc)));
                //printf("flba %i\n", flba * 2048);

                s.st_size-= fsize;
            }

            if(verbose) printf("\r                             \r");

            fclose(fp2);

        }

    closedir (dir);
    }

    path1[len1] = 0;
    
    // iteration
    for(n = 1; n < cur_isop; n++)
        if(directory_iso[n].parent == level + 1) {
            int ret = build_file_iso(fp, path1, path2, n);
            if(ret < 0) {path2[len2] = 0; return ret;}
    }
    
    path2[len2] = 0;

    if(level == 0) {

        if(toc != flba) {
            //printf("End gap: %i\n", (toc - flba));
            
            int f = (toc - flba);
            int f2 = 0;
            int z = 0;

            memset(sectors, 0, ((f > 128) ? 128 : f ) * 2048);
            
            while(f > 0) { 
                if(f > 128) f2 = 128; else f2 = f;

                int ret = write_split(fp, flba + z, sectors, f2, 1);

                if(ret < 0) {
                   return ret;

                }

                f-= f2;
                z+= f2;
            }
        }

    }
    
    
    return 0;

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

static void fixtitle(char *p)
{
    while(*p) {
        if(*p & 128) *p = 0;
        else
        if(*p == ':' || *p == '?' || *p == '"' || *p == '<' || *p == '>' || *p == '|') *p = '_';
        else
        if(*p == '\\' || *p == '/') *p = '-';
        else
        if(((u8)*p) > 0 && ((u8)*p) < 32) {*p = 0; break;}
        p++;
    }
}


int main(int argc, const char* argv[])
{

    struct stat s;
    char path1[0x420];
    char path2[0x420];
    char title_id[64];
    clock_t t_start, t_finish;

    int a = 0;
    int arg_split = 'n';

    // libc test
    if(sizeof(s.st_size) != 8) {

        printf("Error!: stat st_size must be a 64 bit number!  (size %i)\n\nPress ENTER key to exit\n\n", sizeof(s.st_size));
        get_input_char();
        return -1;
    }

    
    if(argc > 1 && (!strcmp(argv[1], "/?") || !strcmp(argv[1], "--help"))) {

        printf("\nMAKEPS3ISO (c) 2013, Estwald (Hermes)\n\n");
    
        printf("%s", "Usage:\n\n"
               "    makeps3iso                                     -> input datas from the program\n"
               "    makeps3iso <pathfiles>                         -> default ISO name\n"
               "    makeps3iso <pathfiles> <ISO file or folder>    -> file or folder destination\n"
               "    makeps3iso -p0 <pathfiles> <ISO file or folder> -> file or folder destination (frontend)\n"
               "    makeps3iso -s <pathfiles>                      -> split files to 4GB\n"
               "    makeps3iso -s <pathfiles> <ISO file or folder> -> split files to 4GB\n"
               "    makeps3iso -p0 -s <pathfiles> <ISO file or folder> -> split files to 4GB (frontend)\n");
               
        return 0;
    }

    if(argc > 1 && (!strcmp(argv[a + 1], "-s") || !strcmp(argv[a + 1], "-S"))) {a++; arg_split = 'y';}
    if(argc > 1 && (!strcmp(argv[a + 1], "-p0") || !strcmp(argv[a + 1], "-P0"))) {a++; verbose = 0;}
    if(argc > 1 && (!strcmp(argv[a + 1], "-s") || !strcmp(argv[a + 1], "-S"))) {a++; arg_split = 'y';}

    if(verbose) printf("\nMAKEPS3ISO (c) 2013, Estwald (Hermes)\n\n");

    if(argc == 1) {
        printf("Enter Path Folder:\n");
        if(fgets(path1, 0x420, stdin)==0) {
            printf("Error Input Path Folder!\n\nPress ENTER key to exit\n"); get_input_char();return -1;
        }
    } else {if(argc >= (2 + a)) strcpy(path1, argv[1 + a]); else path1[0] = 0;}

    fixpath(path1);

    if(stat(path1, &s)<0 || !(S_ISDIR(s.st_mode))) {printf("Invalid Path Folder!\n\nPress ENTER key to exit\n"); get_input_char();return -1;}

    strcpy(path2, path1);
    strcat(path2, "/PS3_GAME/PARAM.SFO");

    if(parse_param_sfo(path2, title_id, output_name)<0) {
        printf("Error!: PARAM.SFO not found!\n");
        printf("\nPress ENTER key to exit\n");
        get_input_char();
        return -1;
    } else {
        utf8_to_ansiname(output_name, path2, 32);
        path2[32]= 0;
        fixtitle(path2);
        strcat(path2, "-");
        strcat(path2, title_id);
    }

    if(argc == 1) {
        printf("\nEnter ISO filename or path (press enter to default name):\n");
        if(fgets(output_name, 0x420, stdin)==0) {
            printf("Error Input ISO filename or path!\n\nPress ENTER key to exit\n"); get_input_char();return -1;
        }
    } else {
        
        if(argc < 3 + a) output_name[0] = 0; else strcpy(output_name, argv[2 + a]);
    }

    fixpath(output_name);

    // create path for get free space from destination file
    char dest_path[0x420];

    strcpy(dest_path, output_name);
    
    char *p = strrchr(dest_path , '/');
    if(!p) {
        strcpy(dest_path, argv[0]); // try from argv[0]
        fixpath(dest_path);
        p = strrchr(dest_path , '/');
        if(!p) strcpy(dest_path, ".");
    }

    if(p) *p = 0;

    u64 avail = get_disk_free_space(dest_path);

    int nlen = strlen(output_name);

    if(nlen < 1) {strcpy(output_name, path2);/*printf("ISO name too short!\n\nPress ENTER key to exit\n"); get_input_char();return -1;*/}
    else {

         if(stat(output_name, &s) == 0 && (S_ISDIR(s.st_mode))) {
            strcat(output_name, "/"); strcat(output_name, path2);
         }
        
    }

    nlen = strlen(output_name);

    if(nlen < 4 || (strcmp(&output_name[nlen - 4], ".iso") && strcmp(&output_name[nlen - 4], ".ISO"))) {
        strcat(output_name, ".iso");
    }

    if(argc < 2) printf("\nSplit in 4GB parts? (y/n):\n");
    
    iso_split = (arg_split== 'y' || argc >= 2) ? arg_split : get_input_char();

    if(iso_split == 'y' || iso_split == 'Y') iso_split = 1; else iso_split = 0;

    if(iso_split) {if(verbose) printf("Using Split File Mode\n");}

    if(verbose) printf("\n");

    t_start = clock();
  
    cur_isop = 0;

    directory_iso = malloc((MAX_ISO_PATHS + 1) * sizeof(_directory_iso));

    if(!directory_iso) {
        printf("Out of Memory (directory_iso mem)\n");
        printf("\nPress ENTER key to exit\n");
        get_input_char();
        return -1;
    }

    memset(directory_iso, 0, (MAX_ISO_PATHS + 1) * sizeof(_directory_iso));

    lpath = sizeof(struct iso_path_table);
    wpath = sizeof(struct iso_path_table);

    llba0 = 0;
    llba1 = 0;
    wlba0 = 0;
    wlba1 = 0;

    dllba = 0; // dir entries
    dwlba = 0; // dir entriesw
    dlsz = 0; // dir entries size (sectors)
    dwsz = 0; // dir entriesw size (sectors)
    flba = 0; // first lba for files

    u32 flba2 = 0;

    int ret;

    ret = calc_entries(path1, 1);
    if(ret < 0 ) {
        switch(ret) {
            case -1:
                printf("Out of Memory (calc_entries())\n");
                goto err;
            case -444:
                printf("Too much folders (max %i) (calc_entries())\n", MAX_ISO_PATHS);
                goto err;
            case -555:
                printf("Folder Name Too Long (calc_entries())\n");
                goto err;
            case -669:
                printf("Error Input File Not Exists (calc_entries())\n");
            case -666:
                printf("Error Opening Input File (calc_entries())\n");
                goto err;
            case -667:
                printf("Error Writing Output File (calc_entries())\n");
                goto err;
            case -668:
                printf("Error Reading Input File (calc_entries())\n");
                goto err;
            case -777:
                printf("Error Creating Split file (calc_entries())\n");
                goto err;

        }
    }

    /*
    printf("llba0 0x%X - offset: 0x%X\n", llba0, llba0 * 2048);
    printf("llba1 0x%X - offset: 0x%X\n", llba1, llba1 * 2048);
    printf("wlba0 0x%X - offset: 0x%X\n", wlba0, wlba0 * 2048);
    printf("wlba1 0x%X - offset: 0x%X\n", wlba1, wlba1 * 2048);

    printf("\ndllba0 0x%X - offset: 0x%X, size 0x%X\n", dllba, dllba * 2048, dlsz * 2048);
    printf("dwlba0 0x%X - offset: 0x%X, size 0x%X\n", dwlba, dwlba * 2048, dwsz * 2048);
    printf("flba0 0x%X - offset: 0x%X\n", flba, flba * 2048);
    */

    flba2 = flba;

    sectors = malloc((flba > 128) ? flba * 2048 + 2048 : 128 * 2048 + 2048);

    if(!sectors) {
        printf("Out of Memory (sectors mem)\n");
        printf("\nPress ENTER key to exit\n");
        get_input_char();
        goto err;
    }

    memset(sectors, 0, flba * 2048);

    pos_lpath0 = llba0 * 2048;
    pos_lpath1 = llba1 * 2048;
    pos_wpath0 = wlba0 * 2048;
    pos_wpath1 = wlba1 * 2048;

    path2[0] = 0;
    ret = fill_entries(path1, path2, 0);

    if(ret < 0 ) {
        switch(ret) {
            case -1:
                printf("Out of Memory (fill_entries())\n");
                goto err;
            case -555:
                printf("File Name Too Long (fill_entries())\n");
                goto err;
            case -669:
                printf("Error Input File Not Exists (fill_entries())\n");
            case -666:
                printf("Error Opening Input File (fill_entries())\n");
                goto err;
            case -667:
                printf("Error Writing Output File (fill_entries())\n");
                goto err;
            case -668:
                printf("Error Reading Input File (fill_entries())\n");
                goto err;
            case -777:
                printf("Error Creating Split file (fill_entries())\n");
                goto err;
            case -444:
                printf("Error in fill_entries()\n");
                goto err;

        }
    }
   
    #ifdef ALIGNED32SECTORS
    flba= (flba + 31) & ~31;
    #endif
    toc = flba;

    if((((u64)toc) * 2048ULL) > (avail - 0x100000ULL)) {
        printf("Error!: Insufficient Disk Space in Destination\n");
        goto err;
    }

    sectors[0x3] = 1; // one range
    set732((void *) &sectors[0x8], 0); // first unencrypted sector
    set732((void *) &sectors[0xC], toc - 1); // last unencrypted sector

    strcpy((void *) &sectors[0x800], "PlayStation3");

    memset((void *) &sectors[0x810], 32, 0x20);
    memcpy((void *) &sectors[0x810], title_id, 10);

    get_rand(&sectors[0x840], 0x1B0);
    get_rand(&sectors[0x9F0], 0x10);


    struct iso_primary_descriptor *isd = (void  *) &sectors[0x8000];
    struct iso_directory_record * idr;

    isd->type[0]=1;
    memcpy(&isd->id[0],"CD001",5);
    isd->version[0]=1;
    
    memset((void *) &isd->system_id[0],32, 32);
    memcpy((void *) &isd->volume_id[0],"PS3VOLUME                       ",32);

    set733((void *) &isd->volume_space_size[0], toc);
    set723((void *) &isd->volume_set_size[0],1);
    set723((void *) &isd->volume_sequence_number[0],1);
    set723((void *) &isd->logical_block_size[0],2048);

    set733((void *) &isd->path_table_size[0], lpath);
    set731((void *) &isd->type_l_path_table[0], llba0); // lba
    set731((void *) &isd->opt_type_l_path_table[0], 0); //lba
    set732((void *) &isd->type_m_path_table[0], llba1); //lba
    set732((void *) &isd->opt_type_m_path_table[0], 0);//lba

    idr = (struct iso_directory_record *) &isd->root_directory_record;
    idr->length[0]=34;
    idr->ext_attr_length[0]=0;
    set733(&idr->extent[0], directory_iso[0].llba); //lba
    set733(&idr->size[0], directory_iso[0].ldir * 2048); // tamaño
    //setdaterec(&idr->date[0],dd,mm,aa,ho,mi,se);
    struct iso_directory_record * aisdr = (void *) &sectors[directory_iso[0].llba * 2048];
    memcpy(idr->date, aisdr->date, 7);

    idr->flags[0]=2;
    idr->file_unit_size[0]=0;
    idr->interleave[0]=0;
    set723(&idr->volume_sequence_number[0],1); //lba
    idr->name_len[0]=1;
    idr->name[0]=0;

    memset(&isd->volume_set_id[0],32,128);
    memcpy(&isd->volume_set_id[0],"PS3VOLUME", 9);
    memset(&isd->publisher_id[0],32,128);
    memset(&isd->preparer_id[0],32,128);
    memset(&isd->application_id[0],32,128);
    memset(&isd->copyright_file_id[0],32,37);
    memset(&isd->abstract_file_id[0],32,37);
    memset(&isd->bibliographic_file_id,32,37);

    unsigned dd1,mm1,aa1,ho1,mi1,se1;

    time_t t;
    struct tm *tm;

    time(&t);

    tm = localtime(&t);
    dd = tm->tm_mday;
    mm = tm->tm_mon + 1;
    aa = tm->tm_year + 1900;
    ho = tm->tm_hour;
    mi = tm->tm_min;
    se = tm->tm_sec;

    dd1=dd;mm1=mm;aa1=aa;ho1=ho;mi1=mi;se1=se;
    if(se1>59) {se1=0;mi1++;}
    if(mi1>59) {mi1=0;ho1++;}
    if(ho1>23) {ho1=0;dd1++;}
    char fecha[64];
    sprintf(fecha,"%4.4u%2.2u%2.2u%2.2u%2.2u%2.2u00",aa1,mm1,dd1,ho1,mi1,se1);

    memcpy(&isd->creation_date[0], fecha,16);
    memcpy(&isd->modification_date[0],"0000000000000000",16);
    memcpy(&isd->expiration_date[0],"0000000000000000",16);
    memcpy(&isd->effective_date[0],"0000000000000000",16);
    isd->file_structure_version[0]=1;

    int len_string;

    isd = (void  *) &sectors[0x8800];

    isd->type[0] = 2;
    memcpy(&isd->id[0],"CD001",5);
    isd->version[0]=1;
    UTF8_to_UTF16((u8 *) "PS3VOLUME", wstring);
    for(len_string = 0; len_string < 512; len_string++) if(wstring[len_string] == 0) break;
    
    
    memset(&isd->system_id[0],0, 32);
    memset(&isd->volume_id[0],0, 32);
    memcpy(&isd->volume_id[0], wstring, len_string * 2);

    set733((void *) &isd->volume_space_size[0],toc);
    set723((void *) &isd->volume_set_size[0],1);
    isd->unused3[0] = 0x25;
    isd->unused3[1] = 0x2f;
    isd->unused3[2] = 0x40;
    set723((void *) &isd->volume_sequence_number[0],1);
    set723((void *) &isd->logical_block_size[0],2048);

    set733((void *) &isd->path_table_size[0], wpath);
    set731((void *) &isd->type_l_path_table[0], wlba0); // lba
    set731((void *) &isd->opt_type_l_path_table[0], 0); //lba
    set732((void *) &isd->type_m_path_table[0], wlba1); //lba
    set732((void *) &isd->opt_type_m_path_table[0], 0);//lba

    idr = (struct iso_directory_record *) &isd->root_directory_record;
    idr->length[0]=34;
    idr->ext_attr_length[0]=0;
    set733(&idr->extent[0], directory_iso[0].wlba); //lba
    set733(&idr->size[0], directory_iso[0].wdir * 2048); // tamaño
    //setdaterec(&idr->date[0],dd,mm,aa,ho,mi,se);
    aisdr = (void *) &sectors[directory_iso[0].wlba * 2048];
    memcpy(idr->date, aisdr->date, 7);

    idr->flags[0]=2;
    idr->file_unit_size[0]=0;
    idr->interleave[0]=0;
    set723(&idr->volume_sequence_number[0],1); //lba
    idr->name_len[0]=1;
    idr->name[0]=0;

    memset(&isd->volume_set_id[0],0,128);
    memcpy(&isd->volume_set_id[0], wstring, len_string * 2);
    memset(&isd->publisher_id[0],0,128);
    memset(&isd->preparer_id[0],0,128);
    memset(&isd->application_id[0],0,128);
    memset(&isd->copyright_file_id[0],0,37);
    memset(&isd->abstract_file_id[0],0,37);
    memset(&isd->bibliographic_file_id,0,37);
    memcpy(&isd->creation_date[0], fecha,16);
    memcpy(&isd->modification_date[0],"0000000000000000",16);
    memcpy(&isd->expiration_date[0],"0000000000000000",16);
    memcpy(&isd->effective_date[0],"0000000000000000",16);
    isd->file_structure_version[0]=1;

    isd = (void  *) &sectors[0x9000];
    isd->type[0] = 255;
    memcpy(&isd->id[0],"CD001",5);
    

    FILE *fp2 = fopen(output_name, "wb");

    if(fp2) {
        fwrite((void *) sectors, 1, (int) flba2 * 2048, fp2);
        flba = flba2;
        if(!verbose) printf("\rPercent done: %u%% \r", 0);
        int ret = build_file_iso(fp2, path1, path2, 0);

        if(ret!= -777) {
            if(fp2) fclose(fp2); fp2 = NULL;
        }

        if(ret < 0 ) {
            if(!verbose) printf("\n");
            switch(ret) {
                case -1:
                    printf("Out of Memory (build_file_iso())\n");
                    goto err;
                case -555:
                    printf("File Name Too Long (build_file_iso())\n");
                    goto err;
                case -669:
                    printf("Error Input File Not Exists (build_file_iso())\n");
                case -666:
                    printf("Error Opening Input File (build_file_iso())\n");
                    goto err;
                case -667:
                    printf("Error Writing Output File (build_file_iso())\n");
                    goto err;
                case -668:
                    printf("Error Reading Input File (build_file_iso())\n");
                    goto err;
                case -777:
                    printf("Error Creating Split file (build_file_iso())\n");
                    goto err;

            }
        
        } else if(!verbose) printf("\rPercent done: %u%% \n", 100);
    } else {
        printf("Error Creating ISO file %s\n", output_name);
        goto err;
    }
    
    int n;
    for(n = 0; n < cur_isop; n++)
        if(directory_iso[n].name) {free(directory_iso[n].name); directory_iso[n].name = NULL;}
    
    free(directory_iso);
    free(sectors);

    t_finish = clock();    

    if(verbose) printf("Finish!\n\nISO TOC: %i\n\n", toc);
    if(verbose) printf("Total Time (HH:MM:SS): %2.2u:%2.2u:%2.2u.%u\n\n", (u32) ((t_finish - t_start)/(CLOCKS_PER_SEC * 3600)),
        (u32) (((t_finish - t_start)/(CLOCKS_PER_SEC * 60)) % 60), (u32) (((t_finish - t_start)/(CLOCKS_PER_SEC)) % 60),
        (u32) (((t_finish - t_start)/(CLOCKS_PER_SEC/100)) % 100));

    if(argc < 2) {
        printf("\nPress ENTER key to exit\n");
        get_input_char();
    }

    return 0;

err:
    if(directory_iso) {
        int n;

        for(n = 0; n < cur_isop; n++)
            if(directory_iso[n].name) {free(directory_iso[n].name); directory_iso[n].name = NULL;}

        free(directory_iso);
    }

    if(sectors) free(sectors);

    printf("\nPress ENTER key to exit\n");
    get_input_char();

    return -1;
}
