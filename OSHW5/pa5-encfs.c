#define FUSE_USE_VERSION 28
#define HAVE_SETXATTR

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
/* For pread()/pwrite()/memstream */
#define _XOPEN_SOURCE 700
#endif

#include "aes-crypt.h"

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/limits.h>
#include <errno.h>
#include <sys/time.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

//A pointer (state*) to the private data returned by the init function
#define fusedata ((state *)fuse_get_context()->private_data)
#define usage "./pa5-encfs <Key Phrase> <Mirror Directory> <Mount Point>"
#define ENCRYPT 0
#define DECRYPT 1
#define ENCRYPTED "true"
#define UNENCRYPTED "false"
#define XFLAG "user.pa5-encfs.encrypted"

typedef struct{
	char* mirrorDir;
	char* key_str;
} state;

static void xmp_path(char fpath[PATH_MAX], const char *path)
{
	strcpy(fpath, fusedata->mirrorDir);
	strncat(fpath, path, PATH_MAX);
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
	int res;
    char fpath[PATH_MAX];

    xmp_path(fpath, path);
    	
	res = lstat(fpath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_access(const char *path, int mask)
{
	int res;
    char fpath[PATH_MAX];

    xmp_path(fpath, path);	
	res = access(fpath, mask);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readlink(const char *path, char *buf, size_t size)
{
	int res;
    char fpath[PATH_MAX];

    xmp_path(fpath, path);
	res = readlink(fpath, buf, size - 1);
	if (res == -1)
		return -errno;

	buf[res] = '\0';
	return 0;
}


static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;
    char fpath[PATH_MAX];

    xmp_path(fpath, path);
	dp = opendir(fpath);
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

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;
    char fpath[PATH_MAX];

    xmp_path(fpath, path);
	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	if (S_ISREG(mode)) {
		res = open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(path, mode);
	else
		res = mknod(fpath, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
	int res;
    char fpath[PATH_MAX];

    xmp_path(fpath, path);
	res = mkdir(fpath, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_unlink(const char *path)
{
	int res;
    char fpath[PATH_MAX];

    xmp_path(fpath, path);
	res = unlink(fpath);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rmdir(const char *path)
{
	int res;
    char fpath[PATH_MAX];

    xmp_path(fpath, path);
	res = rmdir(fpath);
	if (res == -1)
		return -errno;

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
	int res;

	res = rename(from, to);
	if (res == -1)
		return -errno;

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
	int res;
    char fpath[PATH_MAX];

    xmp_path(fpath, path);
	res = chmod(fpath, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid)
{
	int res;
    char fpath[PATH_MAX];

    xmp_path(fpath, path);
	res = lchown(fpath, uid, gid);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
	int res;
    char fpath[PATH_MAX];

    xmp_path(fpath, path);
	res = truncate(fpath, size);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_utimens(const char *path, const struct timespec ts[2])
{
	int res;
	struct timeval tv[2];

	tv[0].tv_sec = ts[0].tv_sec;
	tv[0].tv_usec = ts[0].tv_nsec / 1000;
	tv[1].tv_sec = ts[1].tv_sec;
	tv[1].tv_usec = ts[1].tv_nsec / 1000;

	res = utimes(path, tv);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	int res;
    char fpath[PATH_MAX];

    xmp_path(fpath, path);
	res = open(fpath, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	int res;
	int action;
	FILE* file;
	FILE* fp;
	char* mem_string;
	size_t mem_size;
	
	ssize_t xattrlen;
	char xattrvalue[6];
	
    char fpath[PATH_MAX];
    
    xmp_path(fpath, path);
	(void) fi;
	
	fp = fopen(fpath, "r");
	if (fp == NULL)
		return -errno;
	
	//opens a string for writing to buffer. Dynamically allocates buffer
	file = open_memstream(&mem_string, &mem_size);
	if (file == NULL)
		return -errno;
	
	
	//see if file is encrypted. If it's, then decrypt by getting extended attribute by name. Else ignore.
	xattrlen = getxattr(fpath, XFLAG, xattrvalue, 8);
	if(xattrlen != -1 && strcmp(xattrvalue, ENCRYPTED) == 0) {
		action = DECRYPT;
	}
	
	if (!do_crypt(fp, file, action, fusedata->key_str))
		fprintf(stderr, "do_crypt failed\n");
		
	fclose(fp);
	//write buffer to file
	fflush(file);
	//write from offset position
	fseek(file, offset, SEEK_SET);
	
	
	res = fread(buf, 1, size, file);
	if (res == -1)
		res = -errno;
	fclose(file);
	return res;
}

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int res;
    char fpath[PATH_MAX];
    int action;
    char* mem_string;
    char xattrvalue[6];
    size_t mem_size;
    FILE* file;
    FILE* fp;
    
    ssize_t xattrlen;
	
    xmp_path(fpath, path);
	(void) fi;
	
	fp = fopen(fpath, "w");
	if (fp == NULL)
		return -errno;

	file = open_memstream(&mem_string, &mem_size);
	if (file == NULL)
		return -errno;
	
	// If encrypted, then decrypt, else ignore.
	xattrlen = getxattr(fpath, XFLAG, xattrvalue, 6);
	if(xattrlen != -1 && strcmp(xattrvalue, ENCRYPTED) == 0) {
		action = DECRYPT;
	}
	
	if (!do_crypt(fp, file, action, fusedata->key_str))
		fprintf(stderr, "do_crypt failed\n");
		
	fclose(fp);
	//write buffer to file
	fflush(file);
	//write from offset position
	fseek(file, offset, SEEK_SET);
	
	
	res = fwrite(buf, 1, size, file);
	if (res == -1)
		res = -errno;
	fflush(file);

    // open file again, encrypt from the
	fp = fopen(fpath, "w");
	fseek(file, 0, SEEK_SET);
	
	action = ENCRYPT;
	if (!do_crypt(file, fp, action, fusedata->key_str))
		fprintf(stderr, "do_crypt failed\n");

	//close all files
	fclose(file);
	fclose(fp);
	
	return res;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
	int res;
    char fpath[PATH_MAX];

    xmp_path(fpath, path);
	res = statvfs(fpath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_create(const char* path, mode_t mode, struct fuse_file_info* fi) {

	FILE* fp;
	FILE* file;
    (void) fi;
    (void) mode;
    char fpath[PATH_MAX];
    int action;
    char* mem_string;
    size_t mem_size;
    
    xmp_path(fpath, path);
    
    fp = fopen(fpath, "w");
    if (fp == NULL)
		return -errno;
	
	//dynamically sized output buffer
	file = open_memstream(&mem_string, &mem_size);
	if (file == NULL)
		return -errno;
	
	//set extended attribute, pass encrypted flag
	if (setxattr(fpath, XFLAG, ENCRYPTED, sizeof(ENCRYPTED), 0) == -1)
		return -errno;

	action = ENCRYPT;
	
	if (!do_crypt(file, fp, action, fusedata->key_str))
		fprintf(stderr, "do_crypt failed!\n");
	
	fclose(file);
	fclose(fp);

    return 0;
}


static int xmp_release(const char *path, struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */
    char fpath[PATH_MAX];

    xmp_path(fpath, path);
	(void) path;
	(void) fi;
	return 0;
}

static int xmp_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */
    char fpath[PATH_MAX];

    xmp_path(fpath, path);
	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}

#ifdef HAVE_SETXATTR
static int xmp_setxattr(const char *path, const char *name, const char *value,
			size_t size, int flags)
{
	char fpath[PATH_MAX];

    xmp_path(fpath, path);
	int res = lsetxattr(fpath, name, value, size, flags);
	if (res == -1)
		return -errno;
	return 0;
}

static int xmp_getxattr(const char *path, const char *name, char *value,
			size_t size)
{
	char fpath[PATH_MAX];

    xmp_path(fpath, path);
	int res = lgetxattr(fpath, name, value, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_listxattr(const char *path, char *list, size_t size)
{
	char fpath[PATH_MAX];

    xmp_path(fpath, path);
	int res = llistxattr(fpath, list, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_removexattr(const char *path, const char *name)
{
    char fpath[PATH_MAX];

    xmp_path(fpath, path);
	int res = lremovexattr(fpath, name);
	if (res == -1)
		return -errno;
	return 0;
}
#endif /* HAVE_SETXATTR */

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
#ifdef HAVE_SETXATTR
	.setxattr	= xmp_setxattr,
	.getxattr	= xmp_getxattr,
	.listxattr	= xmp_listxattr,
	.removexattr	= xmp_removexattr,
#endif
};

int main(int argc, char* argv[]){
	/*Local vars*/
	state* State;

	umask(0);
	
	//gets user identity. should always be successful
    if ((getuid() == 0) || (geteuid() == 0)) {
	    fprintf(stderr, "Running ENCFS as root opens unnacceptable security holes\n");
        return 1;
    }
	if (argc != 4){
		fprintf(stderr, "USAGE %s\n", usage);
		exit(EXIT_FAILURE);
	}
	State = malloc(sizeof(State));
	if (State == NULL){
		perror("Error");
		exit(EXIT_FAILURE);
	}
	State->mirrorDir = realpath(argv[2], NULL);
	State->key_str = argv[1];
	
	//edit arguments so that it mimics fusehello - only takes in two arguments in fuse_main parameter
	argv[1] = argv[3];
	argv[2] = NULL;
	argv[3] = NULL;
	argc -= 2;
	
	return fuse_main(argc, argv, &xmp_oper, State);

}
