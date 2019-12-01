// Test that fork fails gracefully.
// Tiny executable so that the limit can be filling the proc table.

#include "types.h"
#include "stat.h"
#include "user.h"
// #include "proc.h"

#define N  1000

static void
putc(int fd, char c)
{
  write(fd, &c, 1);
}

static void
printint(int fd, int xx, int base, int sgn)
{
  static char digits[] = "0123456789ABCDEF";
  char buf[16];
  int i, neg;
  uint x;

  neg = 0;
  if(sgn && xx < 0){
    neg = 1;
    x = -xx;
  } else {
    x = xx;
  }

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);
  if(neg)
    buf[i++] = '-';

  while(--i >= 0)
    putc(fd, buf[i]);
}

// Print to the given fd. Only understands %d, %x, %p, %s.
void
printf(int fd, const char *fmt, ...)
{
  char *s;
  int c, i, state;
  uint *ap;

  state = 0;
  ap = (uint*)(void*)&fmt + 1;
  for(i = 0; fmt[i]; i++){
    c = fmt[i] & 0xff;
    if(state == 0){
      if(c == '%'){
        state = '%';
      } else {
        putc(fd, c);
      }
    } else if(state == '%'){
      if(c == 'd'){
        printint(fd, *ap, 10, 1);
        ap++;
      } else if(c == 'x' || c == 'p'){
        printint(fd, *ap, 16, 0);
        ap++;
      } else if(c == 'b'){
        printint(fd, *ap, 2, 0);
        ap++;
      } else if(c == 's'){
        s = (char*)*ap;
        ap++;
        if(s == 0)
          s = "(null)";
        while(*s != 0){
          putc(fd, *s);
          s++;
        }
      } else if(c == 'c'){
        putc(fd, *ap);
        ap++;
      } else if(c == '%'){
        putc(fd, c);
      } else {
        // Unknown % sequence.  Print it to draw attention.
        putc(fd, '%');
        putc(fd, c);
      }
      state = 0;
    }
  }
}

int a = 1; //because when vfork, pid change and makes new page

int
getpaddrtest(void)
{
  printf(1, "\n\n--------------[DEBUG] init.c getpaddrtest TESTING VFORK\n");

  int pid, wpid;

  int addr;
  
  addr = getpaddr(&a);
  printf(1, "getpaddrtest: before vfork=> &a:%p, paddr(a):%p\n", &a, addr);

  pid = vfork();
  if(pid < 0){
    printf(1, "getpaddrtest: fork failed\n");
    exit();
  }
  if(pid > 0){
    addr = getpaddr(&a);
  printf(1, "getpaddrtest: parent1=> &a:%p, paddr(a):%p\n", &a, addr);
  }
  if(pid == 0){ //child
    addr = getpaddr(&a);
    printf(1, "getpaddrtest: child  => &a:%p, paddr(a):%p\n", &a, addr);
    exit();
  }
  while((wpid=wait()) >= 0 && wpid != pid)
    printf(1, "getpaddrtest: zombie!\n");

  wait();
  addr = getpaddr(&a);
  printf(1, "getpaddrtest: parent2=> &a:%p, paddr(a):%p\n", &a, addr);



  printf(1, "--------------[DEBUG] getpaddrtest TESTING FINISHED\n\n\n");
  return 0;
}

// int
// vforktest(void)
// {
//   printf(1, "\n\n--------------[DEBUG] init.c vforktest TESTING VFORK\n");

//   int pid, wpid;

//   pid = vfork();
//   if(pid < 0){
//     printf(1, "vforktest: fork failed\n");
//     exit();
//   }
//   if(pid == 0){ //child
//     int addr = getpaddr(&a);
//     printf(1, "vforktest: child  => &a:%p, paddr(a):%p\n", &a, addr);
//     exit();
//   }
//   while((wpid=wait()) >= 0 && wpid != pid)
//     printf(1, "vforktest: zombie!\n");

//   //parent
//   wait();
//   int addr = getpaddr(&a);
//   printf(1, "vforktest: parent => &a:%p, paddr(a):%p\n", &a, addr);



//   printf(1, "--------------[DEBUG] vforktest TESTING FINISHED\n\n\n");
//   return 0;
// }

void
forktest(void)
{
  int n, pid;

  printf(1, "fork test\n");

  for(n=0; n<N; n++){
    pid = vfork();
    if(pid < 0)
      break;
    if(pid == 0)
      exit();
  }

  if(n == N){
    printf(1, "fork claimed to work N times!\n", N);
    exit();
  }

  for(; n > 0; n--){
    if(wait() < 0){
      printf(1, "wait stopped early\n");
      exit();
    }
  }

  if(wait() != -1){
    printf(1, "wait got too many\n");
    exit();
  }

  printf(1, "fork test OK\n");
}

int
main(void)
{
  getpaddrtest(); // getpaddrtest
  forktest(); // mutiple vfork
  exit();
}
