#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#define MAX_CHAR 100000 //max size for key

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Length of key is required!\n");
        exit(1);
    }
    int key_len = atoi(argv[1]);
    char key[MAX_CHAR];
    srand(time(0));
    for (int i = 0; i < key_len; i++)
    {
        int rand_num = rand() % 27 + 1; //generate random number between 1 and 27
        if (rand_num == 27) //if the number is 27, make it a space
        {
            key[i] = 32;
        }
        else //otherwise make it a letter between A and Z
        {
            key[i] = (rand_num + 64);
        }
    }
    key[key_len] = '\n'; //add newline character at the end
    printf("%s", key);
    return 0;
}