			/***********open close lseek pfd ***********/
#include "type.h"
		
int open_file(char *cmd_line)
{
  char *pathname, *input_mode, *temp_cmdline, *temp_pathname, *temp;
  int mode, ino, i, k;
  MINODE *mip;
  
  temp = strtok(cmd_line, " ");
  pathname = strtok(0, " ");
  input_mode = strtok(0, " ");
  mode = atoi(input_mode);
  if(mode >= 0 && mode <= 3)
  	printf("\nfile path = %s mode = %d\n", pathname, mode);
  else 
  {
  	printf("%d is an invalid mode 0|1|2|3 for R|W|RW|APPEND\n", mode);
  	return 0;
  }
  if(pathname[0] == '/') dev = root->dev;
  else dev = running->cwd->dev;
  strcpy(temp_pathname, pathname);
  ino = getino(temp_pathname);
  if(ino) mip = iget(dev, ino);
  else
  {
  	printf("filename %s doesn't exist\n", pathname);
  	return 0;
  }
  if(!S_ISREG(mip->INODE.i_mode))
  {
	printf("%s is not a REGULAR file\n", pathname);
  	return 0;
  }
  
  printf("file found\tino = %d\n", ino);
  
  	
  for(k = 0; k < NFD; k++)
  {	
  	if(running->fd[k] != 0 && running->fd[k]->mptr->ino == ino)
  	{
  		if(running->fd[k]->mode)
  		{
  			printf("Can't open %s because it's currently being modified\n", pathname);
  			return 0;
  		}
  	}
  }
  
  OFT *oftp = (OFT *)malloc(sizeof(OFT));
  oftp->mode = mode;
  oftp->refCount = 1;
  oftp->mptr = mip;
  
  switch(mode){
         case 0 : oftp->offset = 0;     // R: offset = 0
                  break;
         case 1 : m_truncate(mip);        // W: truncate file to 0 size
                  oftp->offset = 0;
                  break;
         case 2 : oftp->offset = 0;     // RW: do NOT truncate file
                  break;
         case 3 : oftp->offset =  mip->INODE.i_size;  // APPEND mode
                  break;
         default: printf("invalid mode\n");
                  return(0);
  }
  
  i = 0; k = 0;
  for(i = 0; i < NFD; i++)
  {
  	if(running->fd[i] == NULL || running->fd[i]->refCount == 0)
  	{
  		running->fd[i] = oftp;
  		printf("oftp allocated at fd[%d]\n", i);
  		break;
  	}
  }
  
  if(mode == 0) oftp->mptr->INODE.i_atime = time(0L);
  else
  {
  	oftp->mptr->INODE.i_atime = time(0L);
  	oftp->mptr->INODE.i_mtime = time(0L);
  }
  printf("inode's time updated to %d\n", oftp->mptr->INODE.i_atime);
  mip->dirty = 1;
  iput(mip);
  
  return i;
}

int m_truncate(MINODE *mip)
{
  int i;
  for(i = 0; i < 12; i++)
  {
  	mip->INODE.i_block[i] = 0;
  }
  mip->INODE.i_size = 0;
  mip->INODE.i_atime = mip->INODE.i_mtime = time(0L);
  return 1;
}

int close_file(int fd)
{
  if(fd < 0 || fd >= NFD)
  {
  	printf("index %d is out of range\n", fd);
  	return 0;
  }
  if(running->fd[fd] == NULL)
  {
  	printf("running->fd[%d] is already empty\n", fd);
  	return 0;
  }
  OFT *oftp = (OFT *)malloc(sizeof(OFT));
  oftp = running->fd[fd];
  running->fd[fd] = 0;
  oftp->refCount--;
  printf("closed running->fd[%d]\n", fd);
  if(oftp->refCount > 0) return 1;
  
  printf("last user of this OFT entry ==> dispose of the minode\n");
  MINODE *mip = oftp->mptr;
  iput(mip);
  
  return 1;
}

int my_lseek(char *cmd_line)
{
  int fd, position, original_position;
  char *fd_char, *position_char, *temp;
  
  temp = strtok(cmd_line, " ");
  fd_char = strtok(0, " ");
  position_char = strtok(0, " ");
  
  fd = atoi(fd_char);
  position = atoi(position_char);
  
  if(fd < 0 || fd >= NFD)
  {
  	printf("index %d is out of range\n", fd);
  	return 0;
  }
  
  OFT *oftp = (OFT *)malloc(sizeof(OFT));
  if(running->fd[fd]==NULL)
  {
  	printf("running->fd[%d] is empty\n", fd);
  	return 0;
  }
  oftp = running->fd[fd];
  if(position < 0 || position >= oftp->mptr->INODE.i_size)
  {
  	printf("%d is an invalid offset amount\n", position);
  	return 0;
  }
  original_position = oftp->offset;
  oftp->offset = position;
  
  return original_position;
}
int pfd()
{
  printf("\tfd\tmode\toffset\tINODE [dev, ino, size]\n");
  OFT *oftp = (OFT *)malloc(sizeof(OFT));
  char mode[10];
  for(int i = 0; i < NFD; i++)
  {
  	
  	if(running->fd[i] != NULL)
  	{
  		oftp = running->fd[i];
  		if(oftp->mode == 0)strcpy(mode, "R");
  		else if(oftp->mode == 1)strcpy(mode, "W");
  		else if(oftp->mode == 2)strcpy(mode, "RW");
  		else if(oftp->mode == 3)strcpy(mode, "APPEND");
  		printf("\t%d\t%s\t%d\t[%d, %d, %d]\n", i, mode, oftp->offset, oftp->mptr->dev, 					oftp->mptr->ino, oftp->mptr->INODE.i_size);
  		memset(mode, 0, 10);
  	}
  }
  
  return 1;
}




