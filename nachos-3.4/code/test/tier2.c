#include "syscall.h"

int
main()
{
    Exec("../test/halt");
    Exit(1);
}
