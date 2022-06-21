#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

#define PADDING (2 * DIRSIZ + 2) // 2 dir entries and a possible "->" for symlink

char*
fmtname(char *path)
{
  static char buf[PADDING + 1];

  int pos = 0;

  for (int i = 0; i < strlen(path) && path[i] != '>'; i++) {
    if (path[i] == '/')
      pos = i + 1;
  }

  char *p = path + pos;
  pos = strlen(p);

  // Return blank-padded name.
  if(pos >= PADDING)
    return p;
  memmove(buf, p, pos);
  memset(buf + pos, ' ', PADDING - pos);
  return buf;
}

void
ls(char *path)
{
  char buf[512], *p, *l;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, O_NODEREFERENCE)) < 0){
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_FILE:
    printf("%s %d %d %l\n", fmtname(path), st.type, st.ino, st.size);
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0) {
        continue;
      }
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf("ls: cannot stat %s\n", buf);
        continue;
      }
      if (st.type == T_SOFT) {
        l = buf + strlen(buf);
        if (readlink(buf, l + 2, 512 - strlen(buf) - 2 - 1) < 0) {
          printf("ls: cannot dereference symbolic link %s\n", buf);
          continue;;
        }
        *l++ = '-';
        *l = '>';
      }
      printf("%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
    }
    break;

  case T_SOFT:
    strcpy(buf, path);
    p = buf + strlen(buf);
    if (readlink(buf, p + 2, 512 - strlen(buf) - 1) < 0) {
      printf("ls: cannot dereference symbolic link %s\n", buf);
      break;
    }
    *p++ = '-';
    *p = '>';
    printf("%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    ls(".");
    exit(0);
  }
  for(i=1; i<argc; i++)
    ls(argv[i]);
  exit(0);
}
