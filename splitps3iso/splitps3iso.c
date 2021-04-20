/* 
    (c) 2021 Bucanero <www.bucanero.com.ar>

    SPLITPS3ISO is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SPLITPS3ISO is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    apayloadlong with SPLITPS3ISO.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

#define SPLIT_SIZE       0xFFFF0000
#define BUFFER_SIZE      0x00010000

int verbose = 1;

#if defined (__MSVCRT__)
#define stat _stati64
#endif

#if !defined(fseeko64)
#define fseeko64 fseek
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


///////////////////////////////////////////////////////////////////////////////////////////////////////////


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

static FILE *fp_split = NULL;
static int split_index = 0;

int main(int argc, const char* argv[])
{
    struct stat s;
    int n, len = 0;
    u32 count = 0;

    char path1[0x420];
    char split_file[0x420];
    char *buffer;

    fp_split = NULL;
    split_index = 0;

    clock_t t_start, t_finish;

    // libc test
    if(sizeof(s.st_size) != 8) {

        printf("Error!: stat st_size must be a 64 bit number!  (size %lu)\n\nPress ENTER key to exit\n\n", sizeof(s.st_size));
        get_input_char();
        return -1;
    }

    if(argc > 1 && (!strcmp(argv[1], "/?") || !strcmp(argv[1], "--help"))) {

        printf("\nSPLITPS3ISO (c) 2021, Bucanero\n\n");

        printf("%s", "Usage:\n\n"
               "    splitps3iso                       -> input data from the program\n"
               "    splitps3iso <ISO file>            -> split ISO image (4Gb)\n");
               
        return 0;
    }

    if(verbose) printf("\nSPLITPS3ISO (c) 2021, Bucanero\n\n");

    if(argc == 1) {
        printf("Enter PS3 ISO to patch:\n");
        if(fgets(path1, 0x420, stdin)==0) {
            printf("Error Input PS3 ISO!\n\nPress ENTER key to exit\n"); get_input_char();return -1;
        }
        printf("\n");
    } else {if(argc >= 2) strcpy(path1, argv[1]); else path1[0] = 0;}

    if(path1[0] == 0) {
         printf("Error: ISO file don't exists!\n\nPress ENTER key to exit\n"); get_input_char();return -1;
    }

    fixpath(path1);
    n = strlen(path1);

    if(n >= 4 && (!strcmp(&path1[n - 4], ".iso") || !strcmp(&path1[n - 4], ".ISO"))) {

        sprintf(split_file, "%s.%d", path1, split_index++);
        if(stat(path1, &s)<0) {
            printf("Error: ISO file don't exists!\n\nPress ENTER key to exit\n"); get_input_char();return -1;
        }

    } else {
        printf("Error: file must be with .iso, .ISO extension\n\nPress ENTER key to exit\n"); get_input_char();return -1;
    }
  
    printf("\n");

    FILE *fp = fopen(path1, "rb+");
    if(!fp) {
        printf("Error!: Cannot open ISO file\n\nPress ENTER key to exit\n\n");
        get_input_char();
        return -1;
    }

    t_start = clock();

    fp_split = fopen(split_file, "wb");
    if(!fp_split) {
        printf("Error!: Cannot open ISO file\n\nPress ENTER key to exit\n\n");
        get_input_char();
        return -1;
    }
    
    buffer = malloc(BUFFER_SIZE);
    
    do
    {
    	len = fread(buffer, 1, BUFFER_SIZE, fp);
    	fwrite(buffer, 1, len, fp_split);
    	
    	count += len;
    	
    	if (count == SPLIT_SIZE)
    	{
    		count = 0;
    		fclose(fp_split);
        	sprintf(split_file, "%s.%d", path1, split_index++);
    		fp_split = fopen(split_file, "wb");
    	}
    }
    while(len == BUFFER_SIZE);
    
    free(buffer);

    if(fp) fclose(fp);
    if(fp_split) {fclose(fp_split); fp_split = NULL;}

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
}
