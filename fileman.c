/*********************************************************************
 *
 * Copyright (C) 2020-2022 David C. Harrison. All right reserved.
 *
 * You may not use, distribute, publish, or modify this code without 
 * the express written permission of the copyright holder.
 *
 ***********************************************************************/

/**
 * Sources:
 * https://man7.org/linux/man-pages/dir_section_2.html
 * https://man7.org/linux/man-pages/man2/write.2.html
 * https://man7.org/linux/man-pages/man2/read.2.html
 * https://man7.org/linux/man-pages/man2/lseek.2.html
 * https://man7.org/linux/man-pages/man2/lstat.2.html
 * https://man7.org/linux/man-pages/man2/chdir.2.html
 * https://codeforwin.org/2018/03/c-program-to-list-all-files-in-a-directory-recursively.html
 **/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>

/*
 * Extended ASCII box drawing characters:
 * 
 * The following code:
 * 
 * printf("CSE130\n");
 * printf("%s%s Assignments\n", TEE, HOR);
 * printf("%s  %s%s Assignment 1\n", VER, TEE, HOR);
 * printf("%s  %s%s Assignment 2\n", VER, TEE, HOR);
 * printf("%s  %s%s Assignment 3\n", VER, TEE, HOR);
 * printf("%s  %s%s Assignment 4\n", VER, TEE, HOR);
 * printf("%s  %s%s Assignment 5\n", VER, TEE, HOR);
 * printf("%s  %s%s Assignment 6\n", VER, ELB, HOR);
 * printf("%s%s Labs\n", ELB, HOR);
 * printf("   %s%s Lab 1\n", TEE, HOR);
 * printf("   %s%s Lab 2\n", TEE, HOR);
 * printf("   %s%s Lab 3\n", ELB, HOR);
 * printf();
 * 
 * Shows this tree:
 * 
 * CSE130
 * ├─ Assignments
 * │  ├─ Assignment 1
 * │  ├─ Assignment 2
 * │  ├─ Assignment 3
 * │  ├─ Assignment 4
 * │  ├─ Assignment 5
 * |  └─ Assignment 6
 * └─ Labs
 *    ├─ Lab 1
 *    ├─ Lab 2
 *    └─ Lab 3
 */
#define TEE "\u251C"  // ├ 
#define HOR "\u2500"  // ─ 
#define VER "\u2502"  // │
#define ELB "\u2514"  // └

/*
 * Read at most SIZE bytes from FNAME starting at FOFFSET into BUF starting 
 * at BOFFSET.
 *
 * RETURN number of bytes read from FNAME into BUF, -1 on error.
 */
size_t fileman_read(
  const char *const fname, 
  const size_t foffset, 
  char *const buf, 
  const size_t boffset, 
  const size_t size) 
{
  int fd = open(fname, O_RDONLY);
  if(fd > 0) //opening file
  {
    //to adjust the offset
    lseek(fd, foffset, SEEK_SET); 
    //to read the file
    int file_rd = read(fd, (char *)(buf + boffset), size);  
    return file_rd;
  }
  close(fd);
  return -1; //file doesn't exist
}

/*
 * Create FNAME and write SIZE bytes from BUF starting at BOFFSET into FNAME
 * starting at FOFFSET.
 * 
 * RETURN number of bytes from BUF written to FNAME, -1 on error or if FNAME
 * already exists
 */
size_t fileman_write(
  const char *const fname, 
  const size_t foffset, 
  const char *const buf, 
  const size_t boffset, 
  const size_t size) 
{
  //open file, print error when write to an existing file, 
  int fd = open(fname, O_WRONLY);
  if(fd != -1)
  {
    close(fd);
    return -1;
  }
  //if not, create one
  fd = open(fname, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR);
  if(fd < 0)
  {
    close(fd);
    return -1;
  }
  //to adjust the offset
  lseek(fd, foffset, SEEK_SET);  
  //write to file
  int file_wd = write(fd, (char *)(buf + boffset), size);  
  if(file_wd < 0 || file_wd != size) 
  {
    close(fd);
    return -1;
  }
  close(fd);
  return file_wd; 
}

/*
 * Append SIZE bytes from BUF to existing FNAME.
 * 
 * RETURN number of bytes from BUF appended to FNAME, -1 on error or if FNAME
 * does not exist
 */
size_t fileman_append(const char *const fname, const char *const buf, const size_t size) 
{
  int fd = open(fname, O_APPEND | O_WRONLY);
  if(fd < 0) 
  {
    close(fd);
    return -1;
  }
  //to append the file
  int file_append = write(fd, buf, size);  
  if(file_append < 0 || file_append != size)
  {
    close(fd);
    return -1;
  }
  close(fd);
  return file_append;
}

/*
 * Copy existing file FSRC to new file FDEST.
 *
 * Do not assume anything about the size of FSRC. 
 * 
 * RETURN number of bytes from FSRC written to FDEST, -1 on error, or if FSRC 
 * does not exists, or if SDEST already exists.
 */
size_t fileman_copy(const char *const fsrc, const char *const fdest) 
{
  int f_src = open(fsrc, O_RDONLY);
  if(f_src < 0)
  {
    close(f_src);
    return -1;
  }
  int f_dest = open(fdest, O_WRONLY);
  if(f_dest != -1)
  {
    close(f_dest);
    return -1;
  }
  f_dest = open(fdest, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR);
  if(f_dest < 0)
  {
    close(f_dest);
    return -1;
  }
  char buf[1024];
  int size_buf = 0;
  int bytes_read = 0;
  do
  {
    bytes_read = read(f_src, buf, 1024);
    int bytes_write = write(f_dest, buf, bytes_read);
    size_buf += bytes_write;
  } while(bytes_read == 1024);
  close(f_src);
  close(f_dest);
  return size_buf;
}

//helper function for fileman_dir for recursion
void get_dir(int fd, const char *dname, int level)
{
  //creating a list to store
  struct dirent **flist;
  chdir(dname); //to go into the next directory
  int entry = scandir(".", &flist, NULL, alphasort);

  for(int i = 0; i < entry; i++)
  {
    struct dirent *add_entry = flist[i];
    if (strcmp(add_entry->d_name, ".") == 0 || strcmp(add_entry->d_name, "..") == 0)
    {
      continue;
    }
    else
    {
      //write level spaces to fd
      for(int i = 0; i < level; i++)
      {
        dprintf(fd, "    ");
      }
      dprintf(fd, "%s", add_entry->d_name);
      dprintf(fd, "\n");
    }
    if(add_entry->d_type == DT_DIR)  //if entry is a directory
    {
      get_dir(fd, add_entry->d_name, level + 1);
      chdir("..");
      free(add_entry);
    }
  }
  free(flist);
}

/*
 * Print a hierachival directory view starting at DNAME to file descriptor FD 
 * as shown in an example below where DNAME == 'data.dir'
 *
 *   data.dir
 *       blbcbuvjjko
 *           lgvoz
 *               jfwbv
 *                   jqlbbb
 *                   yfgwpvax
 *           tcx
 *               jbjfwbv
 *                   demvlgq
 *                   us
 *               zss
 *                   jfwbv
 *                       ahfamnz
 *       vkhqmgwsgd
 *           agmugje
 *               surxeb
 *                   dyjxfseur
 *                   wy
 *           tcx
 */
void fileman_dir(const int fd, const char *const dname) 
{
  dprintf(fd, "%s\n", dname);
	get_dir(fd, dname, 1);
}

//helper function for tree
void get_tree(int fd, const char *dname, int level, bool *ancestors)
{
  //creating a list to store
  struct dirent **flist;
  chdir(dname); //to go into the next directory
  int entry = scandir(".", &flist, NULL, alphasort);

  for(int i = 0; i < entry; i++)
  {
    if(i == entry-1) 
    {
      ancestors[level] = false;
    }
    else 
    {
      ancestors[level] = true;
    }

    struct dirent *add_entry = flist[i];
    if (strcmp(add_entry->d_name, ".") == 0 || strcmp(add_entry->d_name, "..") == 0)
    {
      continue;
    }
    else
    {
      //write level spaces to fd
      for(int j = 0; j <= level; j++)
      {
        if(j == level)
        {
          if(i == entry-1) 
          {
            dprintf(fd, "%s%s%s ", ELB, HOR, HOR);
          }
          else 
          {
            dprintf(fd, "%s%s%s ", TEE, HOR, HOR);
          }
        }
        else 
        {
          if(ancestors[j] == true) 
          {
            dprintf(fd, "%s   ", VER);
          }
          else 
          {
            if(j != 0) {dprintf(fd, "    ");}  
          }
        }
      }
      dprintf(fd, "%s", add_entry->d_name);
      dprintf(fd, "\n");
    }
    if(add_entry->d_type == DT_DIR)  //if entry is a directory
    {
      get_tree(fd, add_entry->d_name, level + 1, ancestors);
      chdir("..");
      free(add_entry);
    }
  }
  free(flist);
}


/*
 * Print a hierachival directory tree view starting at DNAME to file descriptor 
 * FD as shown in an example below where DNAME == 'world'.
 * 
 * Use the extended ASCII box drawing characters TEE, HOR, VER, and ELB.
 *
 *   world
 *   ├── europe
 *   │   ├── france
 *   │   │   └── paris
 *   │   │       ├─- entente
 *   │   │       └── saint-germain
 *   │   └── uk
 *   │       ├── london
 *   │       │   ├── arsenal
 *   │       │   └── chelsea
 *   │       └── manchester
 *   │           └── city
 *   │           └── united
 *   └── usa
 *       ├── ma
 *       │   └── boston
 *       │       ├── bruins
 *       │       └── sox
 *       └── tx
 */
void fileman_tree(const int fd, const char *const dname) 
{
  dprintf(fd, "%s\n", dname);
  bool ancestors[100];
	get_tree(fd, dname, 1, ancestors);
}
