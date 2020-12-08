		/***********************mkdir*************************/

#include "type.h"
	
		
int tst_bit(char *buf, int bit)
{
  // in Chapter 11.3.1
  return buf[bit/8] & (1 << (bit % 8));
}

int set_bit(char *buf, int bit)
{
  // in Chapter 11.3.1
  buf[bit/8] |= (1 << (bit % 8));
}

int decFreeInodes(int dev)
{
  //dec free inodes count in SUPER and GD
  char buf[BLKSIZE];
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count--;
  put_block(dev, 1, buf);
  
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count--;
  put_block(dev, 2, buf);


}

int decFreeBlocks(int dev)
{
  //dec free blocks count in SUPER and GD
  char buf[BLKSIZE];
  
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count--;
  put_block(dev, 1, buf);
  
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count--;
  put_block(dev, 2, buf);
  
}

int incFreeInodes(int dev)
{
  char buf[BLKSIZE];
  // inc free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count++;
  put_block(dev, 1, buf);
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count++;
  put_block(dev, 2, buf);
}

int incFreeBlocks(int dev)
{
  char buf[BLKSIZE];
  // inc free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count++;
  put_block(dev, 1, buf);
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count++;
  put_block(dev, 2, buf);

}
int clr_bit(char *buf, int bit) // clear bit in char buf[BLKSIZE]
{ 
  buf[bit/8] &= ~(1 << (bit%8)); 
}

int ialloc(int dev)  // allocate an inode number from inode_bitmap
{
  int i;
  char buf[BLKSIZE];
  // use imap, ninodes in mount table of dev
  get_block(dev, imap, buf);
  for (i=0; i<ninodes; i++){
	if (tst_bit(buf, i)==0){
		set_bit(buf, i);
		put_block(dev, imap, buf);
		// update free inode count in SUPER and GD
		decFreeInodes(dev);//possibly ignore for now?
		return (i+1);
	}
  }
  printf("Error in ialloc() no more free inodes\n");
  return 0; //out of FREE inodes
}

int balloc(int dev)
{
  int i;
  char buf[BLKSIZE];
  get_block(dev, bmap, buf);
  for(i = 0; i<nblocks; i++){
  	if(tst_bit(buf,i)==0){
  		set_bit(buf, i);
  		put_block(dev, bmap, buf);
  		decFreeBlocks(dev);
  		return i + 1;
  	}
  }
  printf("Error in balloc() no more data blocks\n");
  return 0;
}
 
int idalloc(int dev, int ino)
{
  
  char buf[BLKSIZE];
  if(ino > ninodes){
  	printf("inumber %d out of range\n", ino);
  	return 0;
  }
  //get inode bitmap block
  get_block(dev, imap, buf);
  clr_bit(buf, ino-1);
  //write buf back
  put_block(dev, imap, buf);
  //update free inode count in SUPER and GD
  incFreeInodes(dev);
}

int bdalloc(int dev, int bno)
{
  int i; 
  char buf[BLKSIZE];
  if(bno > nblocks){
  	printf("bnumber %d out of range\n", bno);
  	return 0;
  }
  get_block(dev, bmap, buf);
  clr_bit(buf, bno-1);
  //write buf back
  put_block(dev, bmap, buf);
  //update free block count in SUPER and GD
  //printf("got here\n");
  incFreeBlocks(dev);
  

}
int mymkdir(char *pathname){
  char *temp;
  int pino;
  MINODE *pmip = malloc(sizeof(MINODE));
  strcpy(temp, pathname);
  char *parent = dirname(temp);
  char *child = basename(pathname);
  printf("basename = %s\ndirname = %s\n", child, parent);
  pino = getino(parent);
  pmip = iget(dev, pino);
  if (S_ISDIR(pmip->INODE.i_mode)){ // check DIR type
  	//printf("dirname is a directory\n");
  	
  }
  else
  {
  	printf("error dirname: %s is not a directory\n", parent);
  	return 0;
  }
  
  if(!search(pmip, child)){
  	//printf("the directory does not exist yet\n");
  	if(kmkdir(pmip, child))return 1;
  }else{
  	printf("Error the directory already exisits\n");
  	return 0;
  }
  
  return 0;
  	

}

int kmkdir(MINODE *pmip, char *dir_name){
  int ino = ialloc(dev);
  int bno = balloc(dev);
  
  MINODE *mip = iget(dev, ino);
  INODE *ip = &mip->INODE;
  
  ip->i_mode = 0x41ED; // 040755: DIR type and permissions
  ip->i_uid = running->uid; // owner uid
  ip->i_gid = running->gid; // group Id
  ip->i_size = BLKSIZE; // size in bytes
  ip->i_links_count = 2; // links count=2 because of . and ..
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
  ip->i_blocks = 2; // LINUX: Blocks count in 512-byte chunks
  ip->i_block[0] = bno; // new DIR has one data block
  for(int i = 1; i < 15; i++){
  	ip->i_block[i] = 0;
  }
  mip->dirty = 1; // mark minode dirty
  iput(mip); // write INODE to disk
  
  // creating data block for new DIR containing . and .. entries
  char buf[BLKSIZE];
  DIR *dp = (DIR *)buf;
  char *cp = buf;
  
  //make . entry
  dp->inode = ino;
  dp->rec_len = 12;
  dp->name_len = 1;
  dp->name[0] = '.';
  
  //make .. entry
  cp += dp->rec_len;
  dp = (DIR *)cp;
  
  dp->inode = pmip->ino;
  dp->rec_len = BLKSIZE-12;		//rec_len spans block
  dp->name_len = 2;
  dp->name[0] = dp->name[1] = '.';
  put_block(dev, bno, buf);		//write to blk on disk
  
  
  
  enter_name(pmip, ino, dir_name, 1);
  

  return 1;
}

int enter_name(MINODE *pip, int ino, char *name, int option)
{
  char buf[BLKSIZE];
  DIR *dp;
  char *cp;
  int i;
  for(i = 0; i < 12; i++){
  	if(pip->INODE.i_block[i] == 0) break;
  	get_block(pip->dev, pip->INODE.i_block[i], buf);
  	
  	dp = (DIR *)buf;
  	cp = buf;
  	
  	while((cp + dp->rec_len) < buf + BLKSIZE){
  		cp += dp->rec_len;
  		dp = (DIR *)cp;
  	}
  	int remain = dp->rec_len - (4*((8 + dp->name_len + 3)/4));
  	int need_length = 4*((8 + strlen(name) + 3)/4);
  	
  	if (remain >= need_length){
  		//printf("remain = %d current dir length = %d\n", remain, need_length);
  		dp->rec_len = (4*((8 + dp->name_len + 3)/4));
  		cp += dp->rec_len;
  		dp = (DIR *)cp;
  		
  		dp->inode = ino;
  		dp->rec_len = remain;
  		dp->name_len = strlen(name);
  		
  		strncpy(dp->name, name, dp->name_len);
  		dp->name[strlen(name)] = '\n';
  		
  		
  		if(option){					//if the created file is 									directory
  			INODE *ip = &pip->INODE;		//update parents link count
  			ip->i_links_count++;
  			//printf("parent directory links = %d\n", ip->i_links_count);
  			pip->dirty = 1;
  			iput(pip);
  		}
  		put_block(pip->dev, pip->INODE.i_block[i], buf);
  		
  		return 1;
  	}
  	else{
  		printf("Not enough space in existing data blocks\n");
  	
  	
  	}

  }

}

int creat_file(char *pathname)
{
  char *temp;
  int pino;
  MINODE *pmip = malloc(sizeof(MINODE));
  strcpy(temp, pathname);
  char *parent = dirname(temp);
  char *child = basename(pathname);
  //printf("basename = %s\ndirname = %s\n", child, parent);
  pino = getino(parent);
  pmip = iget(dev, pino);
  if (S_ISDIR(pmip->INODE.i_mode)){ // check DIR type
  	//printf("dirname is a directory\n");
  	
  }else{
  	printf("error dirname: %s is not a directory\n", parent);
  	return 0;
  }
  
  if(!search(pmip, child)){
  	//printf("the directory does not exist yet\n");
  	if(my_creat(pmip, child))return 1;
  }else{
  	printf("Error the directory already exisits\n");
  	return 0;
  }
  
  return 0;


}

int my_creat(MINODE *pip, char *name)
{
  int ino = ialloc(dev);
  int bno = balloc(dev);
  
  MINODE *mip = iget(dev, ino);
  INODE *ip = &mip->INODE;
  
  ip->i_mode = 0x81A4; // 0100644: file type and permissions
  ip->i_uid = running->uid; // owner uid
  ip->i_gid = running->gid; // group Id
  ip->i_size = BLKSIZE; // size in bytes
  ip->i_links_count = 1; // links count=2 because of . and ..
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
  ip->i_blocks = 2; // LINUX: Blocks count in 512-byte chunks
  ip->i_block[0] = bno; // new DIR has one data block
  for(int i = 1; i < 15; i++){
  	ip->i_block[i] = 0;
  }
  mip->dirty = 1; // mark minode dirty
  iput(mip); // write INODE to disk
  
  enter_name(pip, ino, name, 0);


}

int is_empty(MINODE *mip){
  char buf[BLKSIZE];
  get_block(dev, mip->INODE.i_block[0], buf);
  DIR *dp = (DIR*)buf;
  char *cp = buf;
  while(cp + dp->rec_len < buf + BLKSIZE){
	cp += dp->rec_len;
	dp = (DIR *)cp;
  }
  char lastName[32];
  strncpy(lastName, dp->name, dp->name_len);
  lastName[dp->name_len] = 0;
  if(strcmp(lastName, "..") == 0 || strcmp(lastName, "lost+found") == 0){
  	return 1;	//the directory really is empty
  }
  else{
  	printf("Directory is not empty the last file is %s\n", lastName);
  	return 0;
  }
  
  

}
int myrmdir(char *pathname)
{
  
  char temp[32], second_temp[32];
  int pino, ino;
  MINODE *mip = malloc(sizeof(MINODE));
  MINODE *pmip = malloc(sizeof(MINODE));
  
  strcpy(temp, pathname);
  strcpy(second_temp, pathname);
  
  char *path = dirname(temp);
  char *dir_name = basename(second_temp);	//name of directory that will be deleted
  printf("parent directory: %s\ndir_name: %s\n", path, dir_name);
  if(strcmp(pathname, ".")==0 || strcmp(pathname, "..")==0){
  	printf("Sorry, can't delete that directory\n");
  	return 0;
  }
  pino = getino(path);		//gets ino of parent directory
  pmip = iget(dev, pino);	//get's mip of parent 
  
  ino = getino(pathname);	//gets ino of directory to be deleted
  mip = iget(dev, ino);	//get's mip of selected directory
  
  
  
  if(ino){
  	//printf("Selected ino number = %d\n", ino);
  	if (S_ISDIR(mip->INODE.i_mode)){ //selected directory
  	
  		if((pmip->INODE.i_links_count > 2) && !is_empty(mip)){ //directory isn't empty
  			printf("Directory %s is not empty\n", dir_name); 
  			return 0;
  		} 
  		else {	//directory is empty and valid to be deleted 
  			for(int i = 0; i < 12; i++){
  				if(mip->INODE.i_block[i]==0){
  					continue;
  					
  				}
  				bdalloc(mip->dev, mip->INODE.i_block[i]);
  			}
  			idalloc(mip->dev, mip->ino);
  			iput(mip);
  			rm_child(pmip, dir_name);
  		
  		}
  	
  	} 
  	else {	//selected non directory
  		printf("%s is not a directory\n", dir_name);
  		return 0;
  	}
  } 
  else {
  	printf("pathname %s is not an existing directory\n", pathname);
  	return 0;
  }
  
}

int rm_child(MINODE *pip, char *dir_name)
{
  MINODE *last_entry = malloc(sizeof(MINODE));
  char buf[BLKSIZE];
  char lastName[32];
  get_block(dev, pip->INODE.i_block[0], buf);
  DIR *dp = (DIR*)buf;
  char *cp = buf;
  char *prev_cp = buf;
  DIR *prev = (DIR*)buf;
  int deleted_len;
  while(cp + dp->rec_len < buf + BLKSIZE){
  	prev = dp;
	cp += dp->rec_len;
	dp = (DIR *)cp;
	
  }
  
  strncpy(lastName, dp->name, dp->name_len);
  lastName[dp->name_len] = '\0';
  printf("lastName %s, dir_name %s\n", lastName, dir_name);
  if(strcmp(lastName, dir_name)==0){
  	printf("is last dir\n");
  	deleted_len = dp->rec_len;
  	prev->rec_len += dp->rec_len;
  	memcpy(dp, prev, dp->rec_len);
  	pip->INODE.i_links_count--;
  	pip->dirty = 1;
  	iput(pip);
  	put_block(fd, pip->INODE.i_block[0], buf);
  
  }
  else{
  	rmMore(pip, dir_name);
  }
}

int rmMore(MINODE *pip, char *dir_name){
  char buf[BLKSIZE];
  get_block(dev, pip->INODE.i_block[0], buf);
  printf("in rmMore with dir_name %s", dir_name);
  DIR *dp = (DIR*)buf;
  char temp[256];
  char *cp = buf;
  char *dp_char = buf;
  DIR *prev = (DIR*)cp;
  DIR *target = (DIR*)cp;
  int deleted_len, mem_size = 0;
  while(cp + dp->rec_len < buf + BLKSIZE){
  	printf("dp->name = %s\n", dp->name);
  	strncpy(temp, dp->name, dp->name_len);
  	temp[dp->name_len] = 0;
  	if(strcmp(temp, dir_name)==0){
		//target = (DIR *)cp;
		printf("found target name %s\n", dp->name);
		deleted_len = dp->rec_len;
		while(cp + dp->rec_len < buf + BLKSIZE){
			cp += dp->rec_len;
			mem_size += dp->rec_len;
			printf("mem_size = %d\n", mem_size);
			//memcpy(dp, cp, 12);
			dp = (DIR*)cp;
			
		}
		printf("final dp = %s\n",dp->name);
		dp->rec_len += deleted_len;
		printf("mem_size = %d\n", mem_size); 
		cp = dp_char + deleted_len;
		memcpy(dp_char, cp, mem_size);
		break;
			
	}
	else{
		cp += dp->rec_len;
		dp_char += dp->rec_len;
		dp = (DIR *)cp;
		//printf("dir name %s rec_len = %d\n", dp->name, dp->rec_len);
		//printf("looking for %s\n", dir_name);
	}
  }
  /*deleted_len += dp->rec_len;
  prev->rec_len = deleted_len;
  memcpy(dp, prev, dp->rec_len);*/
  pip->INODE.i_links_count--;
  pip->dirty = 1;
  iput(pip);
  put_block(fd, pip->INODE.i_block[0], buf);
  	//now target points to directory to be deleted 
  	//and dp points to last directory



}






