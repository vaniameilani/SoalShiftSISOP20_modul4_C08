#define FUSE_USE_VERSION 28
#include<fuse.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<dirent.h>
#include<errno.h>
#include<sys/time.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdbool.h>

char dirpath[50] = "/home/vaniameith/Documents";
char *infodir = "/home/vaniameith/fs.log";
char key[90] = "9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M.b%rI'7d)o4~VfZ*{#:}ETt$3J-zpc]lnh8,GwP_ND|jO";
int ges = 10;

void write_logI(char *text, char* path)
{
    char* info = "INFO";
	char curtime[30];
    time_t t = time(NULL);
    struct tm* loc_time = localtime(&t);
	strftime(curtime, 30, "%y%m%d-%H:%M:%S", loc_time);
    char log[1000];
    sprintf(log, "%s::%s::%s::%s", info, curtime, text, path);
	FILE *fo = fopen(infodir, "a");  
    fprintf(fo, "%s\n", log);  
    fclose(fo); 
    
}

void write_logW(char *text, char* path)
{
    char* info = "WARNING";
    char curtime[30];
    time_t t = time(NULL);
    struct tm* loc_time = localtime(&t);
	strftime(curtime, 30, "%y%m%d-%H:%M:%S", loc_time);
    char log[1000];
    sprintf(log, "%s::%s::%s::%s", info, curtime, text, path);
	FILE *fo = fopen(infodir, "a");  
    fprintf(fo, "%s\n", log);  
    fclose(fo); 
}

char ext[100000] = "\0";
int id = 0;

void sub_string(char *s, char *sub, int p, int l) {
   int c = 0;
   while (c < l) 
   {
      sub[c] = s[p + c];
      c++;
   }
   sub[c] = '\0';
}

char *encrypt(char* str, bool cek)
{
	int i, j, k = 0;
	char *ext = strrchr(str, '.');
	if(cek && ext != NULL) k = strlen(ext);
	for(i = 0; i < strlen(str) - k; i++)
	{
		for(j = 0; j < strlen(key); j++)
		{
			if(str[i] == key[j]){
				str[i] = key[(j + ges) % strlen(key)];
				break;
			}
		}
	}
	return str;
}

char *decrypt(char* str, bool cek)
{
	int i, j, k = 0;
	char *ext = strrchr(str, '.');
	if(cek && ext != NULL) k = strlen(ext);
	for(i = 0; i < strlen(str) - k; i++)
	{
		for(j = 0; j < strlen(key); j++)
		{
			if(str[i] == key[j]){
				str[i] = key[(j + strlen(key) - ges) % strlen(key)];
				break;
			}
		}
	}
	return str;
}

char *lastPart(char *str)
{
	if(!strcmp(str, "/")) return NULL;
	return strrchr(str, '/') + 1;
}

char *cekPath(char *str)
{
	bool encr;
	int start, id;
	encr = 0; start = 1; 
	id = strchr(str + start, '/') - str - 1;
	char curpos[1024];
	while(id < strlen(str))
	{
		strcpy(curpos, "");
		strncpy(curpos, str + start, id - start + 1);
		curpos[id - start + 1] = '\0';
		if(encr)
		{
			encrypt(curpos, 0);
			strncpy(str + start, curpos, id - start + 1);
		}
		if(!encr && strstr(str + start, "encv1_") == str + start) encr = 1;
		start = id + 2;
		id = strchr(str + start, '/') - str - 1;
	}
	id = strlen(str); id--;
	strncpy(curpos, str + start, id - start + 1);
	curpos[id - start + 1] = '\0';
	if(encr)
	{
		encrypt(curpos, 1);
		strncpy(str + start, curpos, id - start + 1);
	}
	return str;
}

char *mixPath(char *fin, char *str1, const char *str2)
{
	strcpy(fin, str1);
	if(!strcmp(str2, "/")) return fin;
	if(str2[0] != '/')
	{
		fin[strlen(fin) + 1] = '\0';
		fin[strlen(fin)] = '/';
	}
	sprintf(fin, "%s%s", fin, str2);
	return fin;
}

int check_ext(char* file)
{
	id = 0;
	while(id < strlen(file) && file[id] != '.') id++;
	memset(ext, 0, sizeof(ext));
	strcpy(ext, file + id);
	return id;
}

int encrFolder(char *str)
{
	int ans;
	char *fi = strtok(str, "/");
	ans = 0;
	while(fi)
	{
		char sub[1024];
		sub_string(fi, sub, 0, 6);
		if(!strcmp(sub, "encv1_")) ans |= 1;
		else if(!strcmp(sub, "encv2_")) ans |= 2;
		fi = strtok(NULL, "/");
	}
	return ans;
}

int encrFull(char *str)
{
	int ans;
	char *fi = strtok(str, "/");
	char *sc = strtok(NULL, "/");
	ans = 0;
	while(sc)
	{
		char sub[1024];
		sub_string(fi, sub, 0, 6);
		if(!strcmp(sub, "encv1_")) ans |= 1;
		else if(!strcmp(sub, "encv2_")) ans |= 2;
		fi = sc;
		sc = strtok(NULL, "/");
	}
	return ans;
}

void loopAllEnc1(char *str, int flag)
{
	struct dirent *dp;
	DIR *dir = opendir(str);
	
	if(!dir) return;
	
	while((dp = readdir(dir)) != NULL)
	{
		if(strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
        	char path[2000000], name[1000000], newname[1000000];
        	mixPath(path, str, dp->d_name);
			strcpy(name, dp->d_name);
			if(flag == 1) mixPath(newname, str, encrypt(name, 1));
			else if(flag == -1) mixPath(newname, str, decrypt(name, 1));
			if(dp->d_type == DT_REG) rename(path, newname);
			else if(dp->d_type == DT_DIR)
			{
				rename(path, newname);
				loopAllEnc1(newname, flag);
			}
        }
	}
}

void encrypt1(char *str, int flag)
{
	struct stat add;
	stat(str, &add);
	if(!S_ISDIR(add.st_mode)) return;
	loopAllEnc1(str, flag);
}

void loopAllEnc2(char *str, int flag)
{
	struct dirent *dp;
	DIR *dir = opendir(str);
	
	if(!dir) return;
	
	while((dp = readdir(dir)) != NULL)
	{
		if(strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
			char path[2000000];
        	mixPath(path, str, dp->d_name);
			if(dp->d_type == DT_DIR) loopAllEnc2(path, flag);
			else if(dp->d_type == DT_REG)
			{
				if(flag == 1) split(path);
				if(flag == -1) combine(path);
			}
        }
	}
}

void encrypt2(char *str, int flag)
{
	struct stat add;
	stat(str, &add);
	if(!S_ISDIR(add.st_mode)) return;
	loopAllEnc2(str, flag);
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
	int res;
	char fpath[1000];
	mixPath(fpath, dirpath, path);
	res = lstat(cekPath(fpath), stbuf);
	write_logI("LS", fpath);
	if (res == -1) return -errno;
	return 0;
}

static int xmp_access(const char *path, int mask)
{
	int res;
	char fpath[1000];
	mixPath(fpath, dirpath, path);
	res = access(cekPath(fpath), mask);
	if (res == -1) return -errno;
	return 0;
}

static int xmp_readlink(const char *path, char *buf, size_t size)
{
	int res;
	char fpath[1000];
	mixPath(fpath, dirpath, path);
	res = readlink(cekPath(fpath), buf, size - 1);
	if (res == -1) return -errno;
	buf[res] = '\0';
	return 0;
}


static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	char fpath[1000];
	mixPath(fpath, dirpath, path);
	int res = 0;
	
	DIR *dp;
	struct dirent *de;
	(void) offset;
	(void) fi;
	dp = opendir(cekPath(fpath));
	if (dp == NULL) return -errno;
	
	int flag = encrFolder(fpath);
	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;		
		st.st_mode = de->d_type << 12;
		char nama[1000000];
		strcpy(nama, de->d_name);
		if(flag == 1)
		{
			if(de->d_type == DT_REG) decrypt(nama, 1);
			else if(de->d_type == DT_DIR && strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) decrypt(nama, 0);
			res = (filler(buf, nama, &st, 0));
			if(res!=0) break;
		}
		else
		{
			res = (filler(buf, nama, &st, 0));
			if(res!=0) break;
		}
	}
	closedir(dp);
    write_logI("CD", fpath);
	return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
	char fpath[1000];
	mixPath(fpath, dirpath, path);
	cekPath(fpath);
	int res;
	
	if (S_ISREG(mode)) {
		res = open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0) res = close(res);
	} else if (S_ISFIFO(mode)) res = mkfifo(fpath, mode);
	else res = mknod(fpath, mode, rdev);
	if (res == -1) return -errno;
	
    write_logI("CREATE", fpath);
	return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
	char fpath[1000];
	mixPath(fpath, dirpath, path);
	
	int res;

	res = mkdir(cekPath(fpath), mode);
	if (res == -1) return -errno;
	
    char cek_substr[1024];
    if(lastPart(fpath) == 0) return 0;
    char filePath[1000000];
    strcpy(filePath, lastPart(fpath));
    sub_string(filePath, cek_substr, 0, 6);
	if(strcmp(cek_substr, "encv1_") == 0) //folder encrypt1
	{
		encrypt1(fpath, 1);	
	}
	else if(strcmp(cek_substr, "encv2_") == 0) //folder encrypt2
	{
		encrypt2(fpath, 1);
	}
	write_logI("MKDIR", fpath);
	return 0;
}

static int xmp_unlink(const char *path)
{
	char fpath[1000];
	mixPath(fpath, dirpath, path);
	int res;

	res = unlink(cekPath(fpath));
    write_logW("REMOVE", fpath);
	if (res == -1) return -errno;
	return 0;
}

static int xmp_rmdir(const char *path)
{
	char fpath[1000];
	mixPath(fpath, dirpath, path);
	int res;

	res = rmdir(cekPath(fpath));
    write_logW("RMDIR", fpath);
	if (res == -1) return -errno;
	return 0;
}

static int xmp_symlink(const char *from, const char *to)
{
	int res;

	res = symlink(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rename(const char *from, const char *to)
{    
    char ffrom[1000];
	mixPath(ffrom, dirpath, from);
	
    char fto[1000];
	mixPath(fto, dirpath, to);
	
	int res;

	res = rename(cekPath(ffrom), cekPath(fto));
	
	if (res == -1)
		return -errno;
	
	int fromm = 0, too = 0;
	char cek_substr[1024], cek2[1024];
    if(lastPart(ffrom) == 0) return 0;
    char filePath[1000000];
    strcpy(filePath, lastPart(ffrom));
    sub_string(filePath, cek_substr, 0, 6);
	if(strcmp(cek_substr, "encv1_") == 0) //folder encrypt1
	{
		fromm = 1;
	}
	else if(strcmp(cek_substr, "encv2_") == 0) //folder encrypt2
	{
		fromm = 2;
	}
	
    if(lastPart(fto) == 0) return 0;
    strcpy(filePath, lastPart(fto));
    sub_string(filePath, cek_substr, 0, 6);
	if(strcmp(cek2, "encv1_") == 0) //folder decrypt1
	{
		too = 1;
	}
	else if(strcmp(cek2, "encv2_") == 0) //folder decrypt2
	{
		too = 2;
	}
	
	if(fromm == 0 && too == 1) encrypt1(fto, 1);
	else if(fromm == 0 && too == 2) encrypt2(fto, 1);
	else if(fromm == 1 && too != 1) encrypt1(fto, -1);
	else if(fromm == 1 && too == 2) encrypt2(fto, 1);
	else if(fromm == 2 && too != 1) encrypt1(fto, -1);
	else if(fromm == 2 && too == 2) encrypt2(fto, 1);
	write_logI("MOVE", ffrom);

	return 0;
}

static int xmp_link(const char *from, const char *to)
{
	int res;

	res = link(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chmod(const char *path, mode_t mode)
{
	char fpath[1000];
	mixPath(fpath, dirpath, path);
	int res;

	res = chmod(cekPath(fpath), mode);
	write_logI("CHMOD", fpath);
	if (res == -1) return -errno;
	return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid)
{
	char fpath[1000];
	mixPath(fpath, dirpath, path);
	int res;

	res = lchown(cekPath(fpath), uid, gid);
	write_logI("CHOWN", fpath);
	if (res == -1) return -errno;
	return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
	char fpath[1000];
	mixPath(fpath, dirpath, path);
	int res;

	res = truncate(cekPath(fpath), size);
    write_logI("TRUNCATE", fpath);
	if (res == -1) return -errno;
	return 0;
}

static int xmp_utimens(const char *path, const struct timespec ts[2])
{
	char fpath[1000];
	mixPath(fpath, dirpath, path);
	int res;
	struct timeval tv[2];

	tv[0].tv_sec = ts[0].tv_sec;
	tv[0].tv_usec = ts[0].tv_nsec / 1000;
	tv[1].tv_sec = ts[1].tv_sec;
	tv[1].tv_usec = ts[1].tv_nsec / 1000;

	res = utimes(cekPath(fpath), tv);
    write_logI("UTIMENS", fpath);
	if (res == -1) return -errno;
	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	char fpath[1000];
	mixPath(fpath, dirpath, path);
	int res;

	res = open(cekPath(fpath), fi->flags);
    write_logI("OPEN", fpath);
	if (res == -1) return -errno;
	close(res);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	char fpath[1000];
	mixPath(fpath, dirpath, path);
	int fd = 0;
	int res = 0;

	(void) fi;
	fd = open(cekPath(fpath), O_RDONLY);
	if (fd == -1) return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1) res = -errno;
	close(fd);
	
    write_logI("CAT", fpath);
	return res;
}

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	char fpath[1000];
	mixPath(fpath, dirpath, path);
	int fd;
	int res;

	(void) fi;
	fd = open(cekPath(fpath), O_WRONLY);
	if (fd == -1) return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1) res = -errno;

    write_logI("WRITE", fpath);
	close(fd);
	return res;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
	char fpath[1000];
	mixPath(fpath, dirpath, path);
	int res;

	res = statvfs(cekPath(fpath), stbuf);
	if (res == -1) return -errno;
	return 0;
}

static int xmp_create(const char* path, mode_t mode, struct fuse_file_info* fi) 
{
	char fpath[1000];
	mixPath(fpath, dirpath, path);
    (void) fi;

    int res;
    res = creat(cekPath(fpath), mode);
    if(res == -1) return -errno;
	
    write_logI("CREAT", fpath);
    close(res);
    return 0;
}


static int xmp_release(const char *path, struct fuse_file_info *fi)
{
	(void) path;
	(void) fi;
	return 0;
}

static int xmp_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi)
{
	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}


static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.access		= xmp_access,
	.readlink	= xmp_readlink,
	.readdir	= xmp_readdir,
	.mknod		= xmp_mknod,
	.mkdir		= xmp_mkdir,
	.symlink	= xmp_symlink,
	.unlink		= xmp_unlink,
	.rmdir		= xmp_rmdir,
	.rename		= xmp_rename,
	.link		= xmp_link,
	.chmod		= xmp_chmod,
	.chown		= xmp_chown,
	.truncate	= xmp_truncate,
	.utimens	= xmp_utimens,
	.open		= xmp_open,
	.read		= xmp_read,
	.write		= xmp_write,
	.statfs		= xmp_statfs,
	.create     = xmp_create,
	.release	= xmp_release,
	.fsync		= xmp_fsync,
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}
