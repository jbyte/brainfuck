#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*#include <windows.h>*/
/*#include <unistd.h>*/

#define DEF_MEMORY 1024
#define VERSION "1.0"
#define TRUE 1
#define FALSE 0

void print_help();
void write_mem_to_file(short *mem, int len, FILE *file);
void write_inst_to_file(char c, FILE *file);
char *load_prog(FILE *f);
int find_match(char *prog, int num);

void execute(char *prog, int memlen, short *mem, int debug, FILE *debug_fp);
void exec_move_right(int *mp, int memlen);
void exec_move_left(int *mp, int memlen);
void exec_add(int mp, short *mem);
void exec_sub(int mp, short *mem);
void exec_write(int mp, short *mem);
void exec_read(int mp, short *mem);
int exec_loop(int pp, char *prog, int *mp, int memlen, short *mem, int debug, FILE *debug_fp);

int main(int argc, const char *argv[]){
    FILE *fp = NULL;
    FILE *debug_fp = NULL;
    short *mem = (short*)calloc(DEF_MEMORY,sizeof(short));
    int val = 0;
    int debug = FALSE;

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
            char *tmp = (char*)malloc(strlen(argv[i+1])*sizeof(char));
            i++;
            strcpy(tmp,argv[i]);
            val = atoi(tmp);
            if(val){
                mem = (short*)calloc(val,sizeof(short));
            }else {
                printf("The number of bytes you have specified is not valid.\n");
            }
        }else if(strcmp(argv[i],"--debug")==0){
            debug = TRUE;
            i++;
            debug_fp = fopen(argv[i],"w");
        }else if(strcmp(argv[i],"--version")==0) {
            printf("bfuck %s\n",VERSION);
        }else if(argc-1>0 && (fp=fopen(argv[i],"rb"))) {
        }else{
            if(argv[i][0]=='-'){
                printf("Unknown flag: %s\n",argv[i]);
            }else{
                printf("Unknown file: %s\n",argv[i]);
            }
            print_help();
            return 1;
        }
    }

    if (fp==NULL) {
        printf("You have not specified a file.\n");
        return 1;
    }else{
        val = (val>=64) ? val : DEF_MEMORY;

        char *prog = load_prog(fp);
        fclose(fp);

        execute(prog,val,mem,debug,debug_fp);
    }

    if(debug_fp!=NULL){
        fclose(debug_fp);
    }

    free(mem);
    return 0;
}

void execute(char *prog, int memlen, short *mem, int debug, FILE *debug_fp){
    int pp = 0;
    int mp = 0;
    printf("%s\n",prog);

    const char instrs[] = "<>+-[],.";

    while(pp<strlen(prog)){
        char tmp = prog[pp];
        switch(prog[pp]){
            case '>':
                exec_move_right(&mp,memlen);
                pp++;
                break;
            case '<':
                exec_move_left(&mp,memlen);
                pp++;
                break;
            case '+':
                exec_add(mp,mem);
                pp++;
                break;
            case '-':
                exec_sub(mp,mem);
                pp++;
                break;
            case '.':
                exec_write(mp,mem);
                pp++;
                break;
            case ',':
                exec_read(mp,mem);
                pp++;
                break;
            case '[':
                pp = exec_loop(pp,prog,&mp,memlen,mem,debug,debug_fp);
                break;
            case ']':
                printf("Unmached \']\'\n");
                exit(1);
            default:
                pp++;
        }
        if(debug && strchr(instrs,tmp)!=NULL){
            write_inst_to_file(tmp,debug_fp);
            if(tmp=='+' || tmp=='-'){
                write_mem_to_file(mem,memlen,debug_fp);
            }
        }
    }
}

void exec_move_right(int *mp, int memlen){
    if(*mp<memlen) (*mp)++;
    else (*mp) = 0;
}
void exec_move_left(int *mp, int memlen){
    if(*mp>0) (*mp)--;
    else (*mp) = memlen -1;
}
void exec_add(int mp, short *mem){
    mem[mp]++;
}
void exec_sub(int mp, short *mem){
    mem[mp]--;
}
void exec_write(int mp, short *mem){
    putchar(mem[mp]);
    /*sleep(1);*/
    /*printf("Every day im printing...\n");*/
}
void exec_read(int mp, short *mem){
    mem[mp] = getchar();
}
int exec_loop(int pp, char *prog, int *mp, int memlen, short *mem, int debug, FILE *debug_fp){
    int tmp = pp;
    int match = find_match(prog,pp);
    /*printf("curr_pos: %d, match: %d\n",pp,match);*/
    pp++;
    /*sleep(5);*/
    if(match==-1){
        /*printf("%c\n",prog[pp]);*/
        printf("Unmached \'[\'\n");
        exit(1);
    }
    const char instrs[] = "<>+-[],.";
    if(mem[*mp]==0) return match+1;
    else{
        while(pp<=match){
            char tmpc = prog[pp];
            switch(prog[pp]){
                case '>':
                    exec_move_right(mp,memlen);
                    pp++;
                    break;
                case '<':
                    exec_move_left(mp,memlen);
                    pp++;
                    break;
                case '+':
                    exec_add(*mp,mem);
                    pp++;
                    break;
                case '-':
                    exec_sub(*mp,mem);
                    pp++;
                    break;
                case '.':
                    exec_write(*mp,mem);
                    pp++;
                    break;
                case ',':
                    exec_read(*mp,mem);
                    pp++;
                    break;
                case '[':
                    pp = exec_loop(pp,prog,mp,memlen,mem,debug,debug_fp);
                    break;
                case ']':
                    if(mem[*mp]!=0) pp = tmp;
                    else pp++;
                    break;
                default:
                    pp++;
                    continue;
            }
            if(debug && strchr(instrs,tmpc)!=NULL){
                write_inst_to_file(tmpc,debug_fp);
                if(tmpc=='+' || tmpc=='-'){
                    write_mem_to_file(mem,memlen,debug_fp);
                }
            }
        }
        return pp;
    }
}

int find_match(char *prog, int num){
    int count = 1;
    int i = num + 1;
    for(; i<strlen(prog); i++){
        if(prog[i]=='[') count++;
        if(prog[i]==']') count--;
        if(count==0) return i;
    }
    return -1;
}

char *load_prog(FILE *f){
    fseek(f,0,SEEK_END);
    long fsize = ftell(f);
    fseek(f,0,SEEK_SET);

    char *str = (char*)malloc((fsize+1)*sizeof(char));
    fread(str,fsize,1,f);

    str[fsize] = 0;
    return str;
}

void print_help(){
    printf("Usage:\n");
    printf("\t\tbfuck [options] <file>\n");
    printf("Options:\n");
    printf("\t\t%*s%*s\n",-25,"--help",-125,"displays the help");
    printf("\t\t%*s%*s\n",-25,"--version",-125,"displays the version");
    printf("\t\t%*s%*s\n",-25,"--debug",-125,"writes the debug info into a file");
    printf("\t\t%*s%*s\n",-25,"--memory n",-125,"sets the data memory to n Bytes (default 1024, minimum 64)");
}

void write_mem_to_file(short *mem, int len, FILE *file){
    fprintf(file, "Memory:\n");
    int i = 0;
    while(i<len){
        if(i%100==0) fprintf(file, "\n");
        fprintf(file, "%2X ", mem[i]);
        i++;
    }
    fprintf(file, "\n");
}

void write_inst_to_file(char c, FILE *file){
    fprintf(file, "Instruction : %c\n", c);
}
