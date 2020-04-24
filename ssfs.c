#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

static  const  char *dirpath = "/home/vaniameith/Documents";

char key[90] = "9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M.b%rI'7d)o4~VfZ{#:}ETt$3J-zpc]lnh8,GwP_ND|jO";
int ges = 10;

void encr(char str)
{
    int i, j;
    if(!strcmp(str, ".") || !strcmp(str, "..")) return;
    for(i = 0; i < strlen(str); i++)
    {
        for(j = 0; j < strlen(key); j++){
            // printf("%c",key[j]);
            if(str[i] == key[j]){
                str[i] = key[(j + ges) % strlen(key)];
                break;
            }
        }
    }
}

void decr(char* str)
{
    int i, j;
    if(!strcmp(str, ".") || !strcmp(str, "..")) return;
    for(i = 0 ; i < strlen(str); i++)
    {
        for(j = 0; j < strlen(key); j++){
            // printf("%c",key[j]);
            if(str[i] == key[j]){
                str[i] = key[(j + strlen(key) - ges) % strlen(key)];
                break;
            }
        }
    }
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
  int res;
  char fpath[1000];
  char name[1000];

  sprintf(name,"%s",path);
  enc(name);
  sprintf(fpath, "%s%s",dirpath,name);
//    printf("getattr custom %s\n", fpath);
  res = lstat(fpath, stbuf);
    if (res != 0)
    {
        return -ENOENT;
    }

    return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
            struct fuse_file_info *fi)
{
    char fpath[1000];
    char name[1000];
    if (strcmp(path, "/") == 0){
        sprintf(fpath, "%s", dirpath);
    }
    else
    {
        sprintf(name,"%s",path);
        enc(name);
        sprintf(fpath, "%s%s",dirpath,name);
    }
    int res = 0;
    int fd = 0 ;

    (void) fi;
    fd = open(fpath, O_RDONLY);
    if (fd == -1)
        return -errno;
    res = pread(fd, buf, size, offset);
    if (res == -1)
        res = -errno;

    close(fd);
    return res;
}

// int main()
// {
// //    char coba[100] = "FOTO_PENTING";
//     char coba[100] = "kelincilucu";
//     encr(coba);
//     printf("%s\n", coba);
//     decr(coba);
//     printf("%s\n", coba);
// }

int main(int argc, char *argv[])
{
    umask(0);
    return fuse_main(argc, argv, &xmp_oper, NULL);
}

