#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>


int main(int argc, const char * argv[]){
    int num_lines, i = 0;
    num_lines = atoi(argv[1]);
    close(STDOUT_FILENO);
    open("totry.txt", O_WRONLY | O_TRUNC, 0644);
    printf("%d", num_lines);
    while(i < num_lines)
        printf("\n%d %d %d", ++i, (rand() % 3) + 1, rand() %50);
    return 0;
}