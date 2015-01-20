#include <stdio.h>
#include <stdlib.h>
 
int main(int argc, char** argv)
{
    FILE* input = NULL;
    FILE* output = NULL;
    unsigned long size = 0;
    unsigned long itr = 0;
    unsigned char* buffer;
    int length;
 
    if(argc != 4)
        return 0;

    length = atoi(argv[1]);
    input = fopen(argv[2],"rb");
    output = fopen(argv[3],"wb");


 

    if(!input || !output)
        fprintf(stderr,"could not open file");
    fseek(input,0,SEEK_END);
    size = ftell(input);
    fseek(input,0,SEEK_SET);
 
    buffer = (unsigned char*)malloc(size);
 
    fread(buffer,1,size,input);
    fclose(input);

    if (length == 2) 
    {
        for(itr = 0;itr<size;itr=itr+4)
        {
            fwrite(&buffer[itr + 1],1,1,output);
            fwrite(&buffer[itr + 0],1,1,output);
            fwrite(&buffer[itr + 3],1,1,output);
            fwrite(&buffer[itr + 2],1,1,output);
        }
    }
    else if (length == 4)
    {
        for(itr = 0;itr<size;itr=itr+4)
        {
            fwrite(&buffer[itr + 3],1,1,output);
            fwrite(&buffer[itr + 2],1,1,output);
            fwrite(&buffer[itr + 1],1,1,output);
            fwrite(&buffer[itr + 0],1,1,output);
        }

    } 
    else 
    {
        fprintf(stderr, "byte's length may be 2 or 4\n");
    }

    fclose(output);
    return 0;
}


