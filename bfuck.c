/**************************** includes **************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*#include <windows.h>*/
/*#include <unistd.h>*/

/**************************** const definitions *****************************/
#define DEF_MEMORY 1024     //default program data memory
#define VERSION "1.0"
#define TRUE 1
#define FALSE 0

/*************************** type and struct definitions *******************/
typedef struct _stack{      // a linked-list implementation of a stack of ints
    int elem;
    struct _stack *next;
}stack;

/*************************** function prototypes ***************************/
void print_help();                                              // interpreter help
// debug help functions
void write_mem_to_file(short *mem, int len, FILE *file);        // if flag --debug, writes memory to file
void write_inst_to_file(char c, FILE *file);                    // if flag --debug, writes current instructio to file

// helper functions for the execution
char *load_prog(FILE *f);                   // loading the contents of a file to a string
int find_match(char *prog, int num);        // find matching square brackets (brainfuck loop)

// stack functions
*stack stack_init();                        // returns a new stack
void stack_push(*stack stck, int elem);     // push an element on the stack
int stack_pop(*stack stck);                 // pop an element from the stack

// brainfuck program execution functions
void execute(char *prog, int memlen, short *mem, int debug, FILE *debug_fp);    // the execution loop
void exec_move_right(int *mp, int memlen);                                      // exec '>'
void exec_move_left(int *mp, int memlen);                                       // exec '<'
void exec_add(int mp, short *mem);                                              // exec '+'
void exec_sub(int mp, short *mem);                                              // exec '-'
void exec_write(int mp, short *mem);                                            // exec '.'
void exec_read(int mp, short *mem);                                             // exec ','
// TODO: replace recursive loop implementation with a non-stack-overflowing one
int exec_loop(int pp, char *prog, int *mp, int memlen, short *mem, int debug, FILE *debug_fp);

/***************************** the main program *****************************/
int main(int argc, const char *argv[]){
    FILE *fp = NULL;
    FILE *debug_fp = NULL;
    // TODO: probably unsigned short?
    short *mem = (short*)calloc(DEF_MEMORY,sizeof(short));
    int val = 0;
    int debug = FALSE;

    if (argc==1) {
        // if called without arguments print help info and exit
        print_help();
        return 1;
    }

    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i],"--help")==0) {
            // handle --help flag
            print_help();
            return 0;
        }else if (strcmp(argv[i],"--memory")==0) { // TODO: handle argv[i+1]==NULL case
            // handle --memory flag
            char *tmp = (char*)malloc(strlen(argv[i+1])*sizeof(char));
            i++;
            strcpy(tmp,argv[i]);
            // create new memory pointer of size val or print error and exit
            val = atoi(tmp);
            if(val){
                mem = (short*)calloc(val,sizeof(short));
            }else {
                printf("The number of bytes you have specified is not valid.\n");
                return 1;
            }
        }else if(strcmp(argv[i],"--debug")==0){ //TODO: handle the 'no specified file' error
            // handle --debug flag
            debug = TRUE;
            i++;
            debug_fp = fopen(argv[i],"w");
        }else if(strcmp(argv[i],"--version")==0) {
            printf("bfuck %s\n",VERSION);
        }else if(argc-1>0 && (fp=fopen(argv[i],"rb"))) {
        }else{
            // handle unknown flags and file names
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
        // TODO: wat? Move memory allocation here
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
    int pp = 0;             // program pointer - index of the current instruction character in program string
    int mp = 0;             // memory pointer - index of the currently accesible memory field
    printf("%s\n",prog);

    const char instrs[] = "<>+-[],."    // TODO: move to global scope

    while(pp<strlen(prog)){
        char tmp = prog[pp];            // temporary variable for storing the program pointer for --debug flag
        switch(prog[pp]){
            // handling of the various brainfuck instructions
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
                // TODO: handle looping w/o recursion
                pp = exec_loop(pp,prog,&mp,memlen,mem,debug,debug_fp);
                break;
            case ']':
                // TODO: handle looping w/o recursion
                printf("Unmached \']\'\n");
                exit(1);
            default:
                pp++;
        }
        if(debug && strchr(instrs,tmp)!=NULL){
            write_inst_to_file(tmp,debug_fp);
            // TODO: add memory print after reading from stdin (or file)
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
}
void exec_read(int mp, short *mem){
    mem[mp] = getchar();
}
// TODO: remove after better loop implementation
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

// TODO: maybe move elsewhere in file?
char *load_prog(FILE *f){
    fseek(f,0,SEEK_END);
    long fsize = ftell(f);
    fseek(f,0,SEEK_SET);

    char *str = (char*)malloc((fsize+1)*sizeof(char));
    fread(str,fsize,1,f);

    str[fsize] = 0;
    return str;
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

void print_help(){
    printf("Usage:\n");
    printf("\t\tbfuck [options] <file>\n");
    printf("Options:\n");
    printf("\t\t%*s%*s\n",-25,"--help",-125,"displays the help");
    printf("\t\t%*s%*s\n",-25,"--version",-125,"displays the version");
    printf("\t\t%*s%*s\n",-25,"--debug",-125,"writes the debug info into a file");
    printf("\t\t%*s%*s\n",-25,"--memory n",-125,"sets the data memory to n Bytes (default 1024, minimum 64)");
    // TODO: maybe add examples?
}
