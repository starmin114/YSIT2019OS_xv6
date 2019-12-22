#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  if(argc != 3){
    printf(2, "Usage: sln old new\n");
    exit();
  }
  if(slink(argv[1], argv[2]) < 0)
    printf(2, "slink %s %s: failed\n", argv[1], argv[2]);
  exit();
}
