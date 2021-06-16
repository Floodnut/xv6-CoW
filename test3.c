#include "types.h"//gsniper777
#include "stat.h"//gsniper777
#include "user.h"//gsniper777

int main(int argc, char **argv)//gsniper777
{//gsniper777
    int before, after;//gsniper777
    int pid;//gsniper777

    printf(1, "TEST3: ");//gsniper777
    before = freemem();//gsniper777
    pid = fork();//gsniper777
    if(pid == 0){//gsniper777
        after = freemem();//gsniper777
        if(before - after == 68)//gsniper777
            printf(1, "OK\n");//gsniper777
        else//gsniper777
            printf(1, "WRONG\n");//gsniper777
        exit();//gsniper777
    }//gsniper777
    else{//gsniper777
        wait();//gsniper777
    }//gsniper777
    exit();//gsniper777
}//gsniper777