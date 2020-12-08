			/*************link***************/
#include "type.h"

int mylink(char *pathname){
  char path_temp[64], *temp, *srcLink, *newFile, *parent, *name;
  int oino, pino;
  MINODE *omip, *pmip;
  
  strcpy(path_temp, pathname);
  temp = strtok(path_temp, " ");
  srcLink = strtok(0, " ");
  newFile = strtok(0, " ");
  printf("srcLink = %s newFile = %s\n", srcLink, newFile);
  oino = getino(srcLink);
  if(oino) omip = iget(dev, oino);
  else 
  {
  	printf("old_file pathname %s does not exist\n", srcLink);
  	return 0;
  }
  if(!S_ISDIR(omip->INODE.i_mode))
  {
  	if(!getino(newFile)){
  		parent = dirname(newFile);
  		name = basename(newFile);
  		pino = getino(parent);
  		pmip = iget(dev, pino);
  		int ino = ialloc(dev);
  		int bno = balloc(dev);
  		
  		//omip->INODE.i_links_count++;
  
  		MINODE *mip = iget(dev, ino);
  		INODE *ip = &mip->INODE;
  		mip->ino = oino;
  		
  		ip->i_mode = 0x81A4; // 0100644: file type and permissions
  		ip->i_uid = running->uid; // owner uid
  		ip->i_gid = running->gid; // group Id
  		ip->i_size = 0; // size in bytes
  		//ip->i_links_count = 1;
  		ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
  		ip->i_blocks = 2; // LINUX: Blocks count in 512-byte chunks
  		ip->i_block[0] = bno; // new DIR has one data block
  		for(int i = 1; i < 15; i++){
  			ip->i_block[i] = 0;
  		}
 
  		enter_name(pmip, mip->ino, name, 0);
  		omip->INODE.i_links_count++;
  		mip->dirty = 1; // mark minode dirty
  		omip->dirty = 1;
  		pmip->dirty = 1;
  		iput(mip); // write INODE to disk
  		iput(omip);
  		iput(pmip);
  	
  	}
  	else
  	{
        	printf("the new_file %s already exists\n", newFile);
        	return 0;
  	}
  }
  else 
  {
  	printf("Error old_file can't be a directory\n");
  	return 0;
  }
}

int my_symlink(char *pathname)
{
  char ncmd[64], old_file[64], new_file[64], temp_pathname[128], buf[BLKSIZE];
  int oino, nino;
  MINODE *omip, *mip;
  INODE *ip;
  
  strcpy(temp_pathname, pathname);
  sscanf(temp_pathname, "%s %s %s", ncmd, old_file, new_file); 	//parses entire cmd line to get the old file and new file pathname
  
  printf("old_file = %s new_file = %s\n", old_file, new_file);
  
  oino = getino(old_file);
  if(oino) omip = iget(dev, oino);
  else
  {
  	printf("%s doesn't exist\n", old_file);
  	return 0;
  }
  if(!(S_ISDIR(omip->INODE.i_mode) || S_ISREG(omip->INODE.i_mode)))
  {
  	printf("%s is an invalid file type\n", old_file);
  	return 0;
  }
  printf("pre creat\n");
  creat_file((char*)new_file);
  printf("post creat\n");
  nino = getino(new_file);
  if(nino)mip = iget(dev, nino);
  else
  {
  	printf("Error with creating or finding %s\n", new_file);
  	return 0;
  }
  

  
  ip = &mip->INODE;		//set ip to mip's inode
  ip->i_size = strlen(old_file) - 1;
  ip->i_mode = 0120000; 	//set new inode mode to link type
  printf("pre strcpy\n");
  strcpy((char*)mip->INODE.i_block, old_file);
  printf("post\n");
  mip->dirty = 1;
  //omip->dirty = 1;
  iput(mip);
  iput(omip);
  //put_block(dev, block, buf);
  
  printf("past everythin\n");
  return 1;
}

char *my_readlink(char *pathname)
{
  char link[64], temp_pathname[64];
  int ino;
  MINODE *mip;
  INODE *ip;
  
  strcpy(temp_pathname, pathname);
  ino = getino(temp_pathname);
  
  if(ino) mip = iget(dev, ino);	//if pathname is an existing file set mip
  else
  {
  	printf("pathname %s doesn't exist\n", pathname);
  	return 0;
  }
  if(mip->INODE.i_mode == 0xA000)	//if file is symlink type
  {
  	return (char*)mip->INODE.i_block;
  }
  else
  {
  	printf("%s is not a symlink file\n", pathname);
  	return 0;
  }
  
}
int my_truncate(INODE *inode)
{
  int i;
  for(i = 0; i < 12; i++)
  {
  	inode->i_block[i] = 0;
  }
  return 1;
}

int my_unlink(char *pathname)
{
  char path_temp[64], buf[BLKSIZE], *parent, *name;
  int ino, pino, block, offset;
  MINODE *mip, *pmip;
  INODE *ip;
  
  strcpy(path_temp, pathname);
  ino = getino(path_temp);
  
  if(ino) mip = iget(dev, ino);		//if file name exists set mip
  else 
  {
  	printf("File name doesn't exist\n");
  	return 0;
  }
  
  if(!S_ISDIR(mip->INODE.i_mode))		//if file name isn't a directory
  {
  	parent = dirname(path_temp);
  	name = basename(pathname);
  	
  	printf("parent = %s file name = %s\n", parent, name);
  	
  	pino = getino(parent);
  	pmip = iget(dev, pino);
  	
  	rm_child(pmip, name);			//remove name from parent directory
  	
  	mip->INODE.i_links_count--;
  	if(mip->INODE.i_links_count > 0) mip->dirty = 1; //write INODE back to disk
  	else
  	{
  		printf("deallocating INODE'S data blocks\n");
  		
  		
  		ip = &mip->INODE;
  		
  		my_truncate(ip);
  		idalloc(dev, ino);
  	}
  	pmip->dirty = 1;
  	iput(mip);
  	iput(pmip);
  	printf("Unlink success\n");
  
  
  }
  else 
  {
  	printf("Error the file name given is a directory, can't unlink a directory\n");
   	return 0;
  }
  
  return 1;
}












