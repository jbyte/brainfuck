#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEF_MEMORY 1024
#define VERSION "0.1.alfa"

void print_help();

int main(int argc, const char *argv[]){
    FILE *fp;

    if (argc==1) {
        print_help();
        return 1;
    }

    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i],"--help")==0) {
            print_help();
            return 0;
        }else if (strcmp(argv[i],"--memory")==0) {
            printf("feature not implemented yet\n");
            return 1;
        }else if (strcmp(argv[i],"--version")==0) {
            printf("bfuck %s\n",VERSION);
        }else if (argc-1>0 && (fp=fopen(argv[i],"r"))) {
            char c;
            while ((c=fgetc(fp))!=EOF) {
                printf("%c",c);
            }
            fclose(fp);
        }else {
            if(argv[i][0]=='-'){
                printf("Unknown command: %s\n",argv[i]);
            }else{
                printf("Unknown file: %s\n",argv[i]);
            }
            print_help();
            return 1;
        }
    }

    return 0;
}

void print_help(){
    printf("Usage:\n");
    printf("\t\tbfuck [options] <file>\n");
    printf("Options:\n");
    printf("\t\t%*s%*s\n",-25,"--help",-125,"displays the help");
    printf("\t\t%*s%*s\n",-25,"--version",-125,"displays the version");
    printf("\t\t%*s%*s\n",-25,"--memory n",-125,"sets the data memory to n Bytes (default 1024)");
}
