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

void logInfo(char* text char* path){
    char* info = "INFO";
    char log[100];
    time_t tm = time(NULL);
    struct tm sTm = *localtime(&tm);

    sprintf(log, "[%s]::[%02d][%02d][%02d]-[%02d]:[%02d]:[%02d]::[%s]::[%s]", info, sTm.tm_year, sTm.tm_mon, sTm.tm_mday, sTm.tm_hour, sTm.tm_min, sTm.tm_sec, text, path);
    FILE* fp;
    fp = fopen("home/ivanar/fs.log", "a");
    fprintf = (fp, "%s", log);
    fclose(fp);
    return 0;
}

void logWarning (char* text char* path){
    char* info = "WARNING";
    char log[100];
    time_t tm = time(NULL);
    struct tm sTm = *localtime(&tm);

    sprintf(log, "[%s]::[%02d][%02d][%02d]-[%02d]:[%02d]:[%02d]::[%s]::[%s]", info, sTm.tm_year, sTm.tm_mon, sTm.tm_mday, sTm.tm_hour, sTm.tm_min, sTm.tm_sec, text, path);
    FILE* fp;
    fp = fopen("home/ivanar/fs.log", "a");
    fprintf = (fp, "%s", log);
    fclose(fp);
    return 0;
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
  logInfo("LS", path);
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
    logInfo("READ", path);
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

static int xmp_rmdir(const char *path)
{
    logWarning("REMOVE_FOLDER", path);
	int res;

	res = rmdir(path);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_unlink(const char *path)
{
    logWarning("REMOVE_FILE", path);
	int res;

	res = unlink(path);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
    logInfo("MKDIR", path);
	int res;

	res = mkdir(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
    logInfo("CREATE", path);
	int res;

	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	if (S_ISREG(mode)) {
		res = open(path, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(path, mode);
	else
		res = mknod(path, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
    logInfo("READDIR", path);
	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(path);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0))
			break;
	}

	closedir(dp);
	return 0;
}

static int xmp_rename(const char *from, const char *to)
{
    logInfo("READDIR", from);
	int res;

	res = rename(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_opendir(const char *path, struct fuse_file_info *fi)
{
    logInfo("OPENDIR", path);
    int res;
    struct xmp_dirp *d = malloc(sizeof(struct xmp_dirp));
    if (d == NULL)
        return -ENOMEM;
 
    d->dp = opendir(path);
    if (d->dp == NULL) {
        res = -errno;
            free(d);
            return res;
    }
    d->offset = 0;
    d->entry = NULL;
 
    fi->fh = (unsigned long) d;
    return 0;
 }

static struct fuse_operations xmp_oper = {
	.getattr = xmp_getattr,
	.readdir = xmp_readdir,
	.read = xmp_read,
	.mkdir = xmp_mkdir,
	.mknod = xmp_mknod,
	.unlink = xmp_unlink,
	.rmdir = xmp_rmdir,
	.rename = xmp_rename,
	.truncate = xmp_truncate,
	.open = xmp_open,
	.read = xmp_read,
	.write = xmp_write,
};

// int main()
// {
// //    char coba[100] = "FOTO_PENTING";
//     char coba[100] = "kelincilucu";
//     encr(coba);
//     printf("%s\n", coba);
//     decr(coba);
//     printf("%s\n", coba);
// }

int  main(int  argc, char *argv[])
{
    mask(0);
    return fuse_main(argc, argv, &xmp_oper, NULL);
}
