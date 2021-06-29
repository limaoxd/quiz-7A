#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM "\t\r\n\a' '"

void lsh_loop(void);
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);
int lsh_execute(char **args);
char *lsh_read_line(void);
char **lsh_split_line(char *line);

char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

char *redirect_str[] = {
    "<",
    ">",
    "|",
};

int (*builtin_func[]) (char **) = {
    &lsh_cd,
    &lsh_help,
    &lsh_exit
};

int main(){
    lsh_loop();

    return EXIT_SUCCESS;
}

void lsh_loop(void)
{
    char *line;
    char **args;
    int status;

    do{
        printf("$ ");
        line = lsh_read_line();
        args = lsh_split_line(line);
        status = lsh_execute(args);

        free(line);
        free(args);
    }while(status);
}

char *lsh_read_line(void)
{
    char *line = NULL;
    ssize_t bufsize = 0;

    if (getline(&line,&bufsize,stdin) == -1){
        if (feof(stdin)){
            exit(EXIT_SUCCESS);
        }else {
            perror("readline");
            exit(EXIT_FAILURE);
        }
    }
    return line;
}

char **lsh_split_line(char *line)
{
    int bufsize = LSH_TOK_BUFSIZE,position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if(!tokens){
        fprintf(stderr,"lsh: allication error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);
    while(token != NULL){
        tokens [position] = token ;
        position++;

        if(position>bufsize){
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens,bufsize*sizeof(char*));
            if(!tokens){
                fprintf(stderr,"lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int lsh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int lsh_cd(char **args)
{
    if(args[1] == NULL){
        fprintf(stderr,"lsh: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0){
            perror("lsh");
        }
    }
    return 1;
}

int lsh_help(char **args)
{
    int i;
    printf("Limaoxd's LSH\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for(i = 0; i < lsh_num_builtins(); i++) {
        printf(" %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}

int lsh_exit(char **args){
    return 0;
}

int lsh_launch(char **args)
{
    pid_t pid,wpid;
    int status;

    pid = fork();
    if(pid == 0){
        if (execvp(args[0],args) == -1){
            perror("lsh");
        }
        else if()
        exit(EXIT_FAILURE);
    } else if (pid < 0){
        perror("lsh");
    } else {
        do{
            wpid = waitpid(pid, &status, WUNTRACED);
        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int chk_redirect(char **args)
{
    int state = 0, i = 0, j = 0;
    int arg_size = sizeof(args) / sizeof(char *), sym_size = sizeof(redirect_str) / sizeof(char *);

    for(;i<arg_size;i++){
        for(;j<sym_size;j++)
            if(strcmp(args[i],redirect_str[j]) == 0){
                state = 1;
            }
        j = 0;
    }
    
    return state;
}

int lsh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < lsh_num_builtins(); i++) {


    if (strcmp(args[0], builtin_str[i]) == 0) {
        return (*builtin_func[i])(args);
    }
  }

  return lsh_launch(args);
}