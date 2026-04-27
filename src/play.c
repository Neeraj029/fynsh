// C program to illustrate
// the strrchr() function
#include <stdio.h>
#include <string.h>

int main() {
    int char_limit = 18;
    int colms = 3;
    char test[] = "hi";
    int sizeoff_name = sizeof(test)/sizeof(char) - 1;
    printf(test);
    if(sizeoff_name<=10){
        for(int i = 0;i<(char_limit - sizeoff_name);i++){
            putchar(' ');
        }
    }
    printf("did it work \n");
    // initializing string
    char *fnames[] = {"fynsh", "d3d10_1core.dll", "dafDockingProvider.dll","sdfasdfff","dsfasd","sdfxx"};
    for(int i = 0;i<sizeof(fnames)/sizeof(fnames[0]);i++){
        printf("%s \t", fnames[i]);
        if((i+1)%colms == 0){
            printf("\n");
        }
    }

    return 0;
}