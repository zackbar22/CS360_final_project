/************* cd_ls_pwd.c file **************/
#include "type.h"

int cd(char *pathname)   
{
  
  printf("chdir %s\n", pathname);
  // READ Chapter 11.7.3 HOW TO chdir
  int c, ino;
  	
  MINODE *mip = malloc(sizeof(MINODE));
  mip = running->cwd;
  ino = getino(pathname);
  
  mip = iget(dev, ino);
  
  if(S_ISDIR(mip->INODE.i_mode))
	{
		
	if(running->cwd == root){	
		printf("mip = root\n");
	}
	
	iput(running->cwd);
	running->cwd = mip;
  }
  else
  {
	printf("Error: tried to cd to non dir\n");
	iput(running->cwd);
	running->cwd = root;
  }
	
}

int ls_file(MINODE *mip, int ino)
{
  printf("ls_file: to be done: READ textbook for HOW TO!!!!\n");
  // READ Chapter 11.7.3 HOW TO ls
}

int ls_dir(MINODE *mip)
{
  printf("ls_dir: list CWD's file names; YOU do it for ls -l\n");

  char buf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;
  
  // Assume DIR has only one data block i_block[0]
  get_block(dev, mip->INODE.i_block[0], buf); 
  dp = (DIR *)buf;
  cp = buf;
  
  while (cp < buf + BLKSIZE){
     strncpy(temp, dp->name, dp->name_len);
     temp[dp->name_len] = 0;
	
     printf("[%d %s]  ", dp->inode, temp); // print [inode# name]

     cp += dp->rec_len;
     dp = (DIR *)cp;
  }
  printf("\n");
}

int ls(char *pathname)  
{
  int c, ino;
  MINODE *mip = malloc(sizeof(MINODE));
  MINODE *pcur = malloc(sizeof(MINODE));
  
  mip = running->cwd;
  
  if(pathname[0]){
  	
  	pcur = running->cwd;
  	
  	ino = getino(pathname);
  	
  	if(name[0]){
  		mip = iget(dev, ino);
	}
  	else
  		mip = root;
  	
  
  }
  if (S_ISDIR(mip->INODE.i_mode)) // check DIR type
  	printdir(mip->INODE);
  else 
  	ls_file(mip, ino);
}

char *pwd(MINODE *wd)
{
  
  if (wd == root){
    printf("/\n");
  
  }
  else
  {
   	char path[32];
   	char finalPath[32];
   	MINODE *mip = malloc(sizeof(MINODE));
   	mip = wd;
   	while(mip->ino != root->ino){
   		
   		int ino = mip->ino;
   		
   		mip = getParent(mip, ino, path);
   		strcat(path, nsearch(mip, ino));
   		strcat(path, "/");
   		//printf("yea\n");
   		/*if(strlen(path) == 0){
   			strcpy(path, temp);
   			//printf("%s\n", temp);
   		}
   		else{
   			strcat(path, temp);
   			//strcpy(path, "");
   			//strcpy(path, temp);
   			//printf("%s\n", path);
   		}*/
   	}
   	char *wordArray[32];	
   	char *s;
   	int i, j, counter = 0;
   	s = strtok(path, "/");
   	
   	while(s){
   		//printf("the char %c", s[strlen(s)]);
   		if(s[strlen(s)] == '\0')
   		{
   			s[strlen(s)] = 0;
   		}
   		wordArray[i] = s;
   		i++;
   		s = strtok(0, "/");
   			
   	}
   	int post = 1;
   	for(i--; i>=0; i--){
   		printf("/");
   		if(i == 0) post = 0;
   		for(j = 0; j<strlen(wordArray[i]) - post; j++){
   			
   			printf("%c", wordArray[i][j]);
   		}
   		
   		
   	}

   		
   	
   		
   	
  }
}

void mypwd(MINODE *wd)
{


}
void printdir(INODE inode){
	char buf[BLKSIZE];
	get_block(dev, inode.i_block[0], buf);
	DIR *dp = (DIR*)buf;
	char *cp = buf;
	printf("\ti_num\tlnk_num\trec_len\tname_len\tname\n");
	while(cp < buf + BLKSIZE){
	    char dir_name[64];
	    strncpy(dir_name, dp->name, dp->name_len);
	    dir_name[dp->name_len] = 0;
	    MINODE* mip = iget(dev, dp->inode);
	    if(mip->INODE.i_mode == 0xA000)
	    	printf("\t%d\t%d\t%d\t%d\t%s->%s\n", dp->inode, mip->INODE.i_links_count, 			dp->rec_len, dp->name_len, dir_name, (char*)mip->INODE.i_block);
	    else
	    	printf("\t%d\t%d\t%d\t%d\t%s\n", dp->inode, mip->INODE.i_links_count, 							dp->rec_len, dp->name_len, dir_name);
	    cp += dp->rec_len;
	    dp = (DIR *)cp;
	    memset(dir_name, 0, 64);
	}
	printf("\n");
}
void printName(INODE ind){
  	char buf[1024];
	get_block(dev,ind.i_block[0], buf);
	DIR *tp = (DIR*)buf;
	//printf("%s\n", tp); 
	


}

