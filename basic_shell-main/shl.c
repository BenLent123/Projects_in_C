#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>

//gcc -o shl shl.c

#define SHLBUFSIZE 1024
#define SHLTOKBUFSIZE 64
#define SHLTOKDELIM " \t\r\n\a"

int shl_cd(char **args);
int shl_help(char **args);
int shl_exit(char **args);
int shl_pwd(char **args);  
int shl_echo(char **args);
int shl_nyan(char **args);



char *shl_readLine(){
    char *line = NULL;
    ssize_t bufsize = 0;
    if(getline(&line,&bufsize,stdin)==-1){
        if(feof(stdin)){
            exit(EXIT_SUCCESS);
        }else{
            perror("readline");
            exit(EXIT_FAILURE);
        }
    }
    return line;
}

char **shl_splitLine(char *line){
    int bufsize = SHLTOKBUFSIZE;
    int pos = 0;
    char **tokens = malloc(bufsize * sizeof(char*));  // Fixed: sizeof(char*)
    char *token;
    
    if(!tokens){  // Fixed: check tokens, not token
        perror("shl token allocation error");
        exit(EXIT_FAILURE);
    }
    
    token = strtok(line, SHLTOKDELIM);
    while(token != NULL){
        tokens[pos] = token;
        pos++;
        
        if(pos >= bufsize){
            bufsize += SHLTOKBUFSIZE;  // Fixed: increase bufsize
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if(!tokens){
                perror("shl allocation error");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, SHLTOKDELIM);
    }
    tokens[pos] = NULL;
    return tokens;
}

int shl_launch(char *args[]){
    pid_t pid, wpid;
    int status;
    pid = fork();
    if(pid==0){
        if(execvp(args[0],args)==-1){
            fprintf(stderr,"shl has not found command-> %s! \n", args[0]); // custom error msg
        }
        exit(EXIT_FAILURE);
    }else if (pid<0){
       perror("shl fork failed");
    }else{
        do{
            wpid = waitpid(pid, &status, WUNTRACED);
        }while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

char *builtin_str[] = {
    "cd",
    "help",
    "exit",
    "pwd",
    "echo",
    "nyan",
};

int (*builtin_func[]) (char **) = {
  &shl_cd,
  &shl_help,
  &shl_exit,
  &shl_pwd,
  &shl_echo,
  &shl_nyan,
};

int shl_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int shl_cd(char **args){
  if (args[1] == NULL) {
    perror("shl: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("shl");
    }
  }
  return 1;
}

int shl_nyan(char **args){
    if(args[1] != NULL){
        perror("no arguments expected");
    } else{
        printf(":3\n");
    }
    
    return 1;
}

int shl_help(char **args){
  int i;
  printf("ben lent's shell -> SHL \n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < shl_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int shl_exit(char **args){
  return 0;
}

int shl_pwd(char **args) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("shl");
    }
    return 1;
}

int shl_echo(char **args) {
    for (int i = 1; args[i] != NULL; i++) {
        printf("%s", args[i]);
        if (args[i + 1] != NULL) printf(" ");
    }
    printf("\n");
    return 1;
}

int shl_exec(char **args){
  int i;

  if (args[0] == NULL) {
    printf("empty command entered \n");// An empty command was entered.
    return 1;
  }

  for (i = 0; i < shl_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return shl_launch(args);
}


void shl_loop(){
    char *line;char **args;int status;
    do{
        printf(">");
        line = shl_readLine();
        args = shl_splitLine(line);
        status = shl_exec(args);
    }while(status);
}

int main (int argc, char *argv[]){

    shl_loop();

    return EXIT_SUCCESS;
}
