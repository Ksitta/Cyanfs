#include "file.h"
#include "defs.h"
#include "fcntl.h"
#include "fs.h"

//The operation performed on the system-level open file table entry after some process closes a file.
void fileclose(struct file *f)
{
	if (f->ref < 1)
		panic("fileclose");
	if (--f->ref > 0) {
		return;
	}

	iput(f->ip);

	f->off = 0;
	f->readable = 0;
	f->writable = 0;
	f->ref = 0;
	f->type = file::FD_NONE;
	free(f);
}

//Show names of all files in the root_dir.
int show_all_files()
{
	return dirls(root_dir());
}

//Create a new empty file based on path and type and return its inode;
//if the file under the path exists, return its inode;
//returns 0 if the type of file to be created is not T_file
static struct inode *create(char *path, short type)
{
	struct inode *ip, *dp;
	dp = root_dir(); //Remember that the root_inode is open in this step,so it needs closing then.
	ivalid(dp);
	if ((ip = dirlookup(dp, path, 0)) != 0) {
		printf("create a exist file\n");
		iput(dp); //Close the root_inode
		ivalid(ip);
		if (type == T_FILE && ip->type == T_FILE)
			return ip;
		iput(ip);
		return 0;
	}
	if ((ip = ialloc(dp->dev, type)) == 0)
		panic("create: ialloc");

	ivalid(ip);
	iupdate(ip);
	if (dirlink(dp, path, ip->inum) < 0)
		panic("create: dirlink");

	iput(dp);
	return ip;
}

//A process creates or opens a file according to its path, returning the file descriptor of the created or opened file.
//If omode is O_CREATE, create a new file
//if omode if the others,open a created file.
struct file * fileopen(char *path, uint64 omode)
{
	int fd;
	struct file *f;
	struct inode *ip;
	if (omode & O_CREATE) {
		ip = create(path, T_FILE);
		if (ip == 0) {
			return NULL;
		}
	} else {
		if ((ip = namei(path)) == 0) {
			return NULL;
		}
		ivalid(ip);
	}
	if (ip->type != T_FILE){
		panic("unsupported file inode type\n");
	}
	f = (struct file *)malloc(sizeof(struct file));

	// only support FD_INODE
	f->type = file::FD_INODE;
	f->off = 0;
	f->ip = ip;
	f->readable = !(omode & O_WRONLY);
	f->writable = (omode & O_WRONLY) || (omode & O_RDWR);
	if ((omode & O_TRUNC) && ip->type == T_FILE) {
		itrunc(ip);
	}
	return f;
}

// Write data to inode.
uint64 inodewrite(struct file *f, char* src, uint64 len)
{
	int r;
	ivalid(f->ip);
	if ((r = writei(f->ip, src, f->off, len)) > 0){
		f->off += r;
	}
	return r;
}

//Read data from inode.
uint64 inoderead(struct file *f, char *dst, uint64 len)
{
	int r;
	ivalid(f->ip);
	if ((r = readi(f->ip, dst, f->off, len)) > 0){
		f->off += r;
	}
	return r;
}