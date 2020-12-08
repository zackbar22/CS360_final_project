/*************** type.h file ************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <ext2fs/ext2_fs.h>
#include <libgen.h>
#include <string.h>
#include <sys/stat.h>


typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

  



#define BLKSIZE 1024
#define SUPERBLOCK	1
#define GDBLOCK	2
#define ROOT_INODE 	2

//Default dir and regular file modes
#define DIR_MODE 	0x41ED
#define FILE_MODE 	0x81AE
#define SUPER_MAGIC	0xEF53
#define SUPER_USER	0

#define INODE_START_POS   10

//Proc status
#define FREE        0
#define READY       1

//file system table sizes
#define NMINODE	100
#define NMTABLE	10
#define NPROC		10
#define NFD		10
#define NOFT		40

//In-memory inodes structure
typedef struct minode{
  INODE INODE;
  int dev, ino;
  int refCount;
  int dirty;
  int mounted;
  struct mntable *mptr;
}MINODE;

//Open File Table
typedef struct oft{
  int  mode;
  int  refCount;
  MINODE *mptr;
  int  offset;
}OFT;

//PROC structure
typedef struct proc{
  struct proc *next;
  int          pid;
  int          ppid;
  int          status;
  int          uid, gid;
  MINODE      *cwd;
  OFT         *fd[NFD];
}PROC;


typedef struct mtable{
  int dev;
  int ninodes;
  int nblocks;
  int free_blocks;
  int free_inodes;
  int bmap;
  int imap;
  int iblok;
  MINODE *mntDirPtr;
  char devName[64];
  char mntName[64];
}MTABLE;

// globals
MINODE minode[NMINODE];
MINODE *root;

PROC   proc[NPROC];
PROC	*running;

char gline[128]; // global for tokenized components
char *name[32];  // assume at most 32 components in pathname
int   nname;         // number of component strings

int fd, dev;
int nblocks, ninodes, bmap, imap, inode_start;


SUPER *sp;
GD    *gp;
INODE *ip;
DIR   *dp; 

char *nsearch(MINODE *mip, int ino);
int get_block(int dev, int blk, char *buf);
int put_block(int dev, int blk, char *buf);
int tokenize(char *pathname);
MINODE *iget(int dev, int ino);
void iput(MINODE *mip);
int search(MINODE *mip, char *name);
int getino(char *pathname);
int findmyname(MINODE *parent, u32 myino, char *myname);
int findino(MINODE *mip, u32 *myino); // myino = ino of . return ino of ..
int cd(char *pathname);
int ls_file(MINODE *mip, int ino);
int ls_dir(MINODE *mip);
int ls(char *pathname);
int mymkdir(char *pathname);
char *pwd(MINODE *wd);
void printdir(INODE ind);
void mypwd(MINODE *wd);
MINODE *getParent(MINODE *mip, int ino, char *path);
void printName(INODE ind);
int tst_bit(char *buf, int bit);
int set_bit(char *buf, int bit);
int ialloc(int dev);
int balloc(int dev);
int enter_name(MINODE *pip, int ino, char *name, int option);
int mymkdir(char *pathname);
int kmkdir(MINODE *pmip, char *dir_name);
int my_creat(MINODE *pip, char *name);
int creat_file(char *pathname);
int myrmdir(char *pathname);
int rm_child(MINODE *pip, char *dir_name);
int idalloc(int dev, int ino);
int bdalloc(int dev, int bno);
int incFreeBlocks(int dev);
int incFreeInodes(int dev);
int rmMore(MINODE *pip, char *dir_name);
int mylink(char *pathname);
int my_unlink(char *pathname);
int my_truncate(INODE *ino);
int my_symlink(char *pathname);
char *my_readlink(char *pathname);
int open_file(char *cmd_line);
int m_truncate(MINODE *mip);
int close_file(int fd);
int pfd();
int my_lseek(char *cmd_line);
