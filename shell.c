#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<math.h>
#include<windows.h>
#include<dirent.h>

#ifdef _WIN32
#include<windows.h>
#define SPHS_TOK_BUFSIZE 64
#define SPHS_TOK_DELIM " \t\r\n\a"
#else
#include<unistd.h>
#include<sys/wait.h>
#endif

char **sphs_token_line(char *line){
    int bufsize=SPHS_TOK_BUFSIZE;
    int position=0;
    char **tokens=malloc(bufsize*sizeof(char*));
    char *token;
    
    if(!tokens){
        fprintf(stderr,"sphs: allocation error\n");
        exit(EXIT_FAILURE);
    }
    
    token=strtok(line,SPHS_TOK_DELIM);
    while(token!=NULL){
        tokens[position]=token;
        position++;
        
        if(position>=bufsize){
            bufsize+=SPHS_TOK_BUFSIZE;
            tokens=realloc(tokens,bufsize*sizeof(char*));
            if(!tokens){
                fprintf(stderr,"sphs: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token=strtok(NULL,SPHS_TOK_DELIM);
    }
    tokens[position]=NULL;
    return tokens;
}

int sphs_launch(char **args){
#ifdef _WIN32
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    char command[1024] = "";
    for (int i = 0; args[i] != NULL; ++i) {
        strcat(command, args[i]);
        strcat(command, " ");
    }
    LPSTR lpCommandLine = command;

    if (!CreateProcess(NULL, lpCommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        fprintf(stderr, "CreateProcess failed (%d).\n", GetLastError());
        return 1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#else
    pid_t pid, wpid;
    int status;
    
    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("sphs");
            exit(EXIT_FAILURE); 
        }
    } else if (pid < 0) {
        perror("sphs");
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
#endif
    return 1;
}

int sphs_help(char **args);
int sphs_exit(char **args);
int sphs_cd(char **args);
int sphs_rm(char **args);
int sphs_rmdir(char **args);
int sphs_ls(char **args);
int sphs_zeta(char **args);
int sphs_path(char **args);
int sphs_exec_path(char **args);

char *builtin_str[] = {
    "cd",
    "help",
    "exit",
    "rm",
    "rmdir",
    "ls",
    "zeta",
    ""
};

int (*builtin_func[]) (char **) = {
    &sphs_cd,
    &sphs_help,
    &sphs_exit,
    &sphs_rm,
    &sphs_rmdir,
    &sphs_ls,
    &sphs_zeta,
    &sphs_exec_path
};


int sphs_num_builtins(){
    return sizeof(builtin_str)/sizeof(char*);
}
int sphs_execute(char **args){
  int i;

  if (args[0] == NULL) {
    return 1;
  }

  for (i = 0; i < sphs_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return sphs_launch(args);
}
char *sphs_read_line(){
    char *line = NULL;
    size_t bufsize = SPHS_TOK_BUFSIZE;
    size_t position = 0;
    int c;

    line = malloc(bufsize * sizeof(char));
    if (!line) {
        fprintf(stderr, "sphs: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        c = getchar();

        if (c == EOF || c == '\n') {
            line[position] = '\0';
            return line;
        } else {
            line[position] = c;
        }
        position++;

        if (position >= bufsize) {
            bufsize += SPHS_TOK_BUFSIZE;
            line = realloc(line, bufsize * sizeof(char));
            if (!line) {
                fprintf(stderr, "sphs: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}
int sphs_cd(char **args){
#ifdef _WIN32
    if (args[1] == NULL) {
        fprintf(stderr, "sphs: expected argument to \"cd\"\n");
    } else {
        if (!SetCurrentDirectory(args[1])) {
            fprintf(stderr, "sphs: cd: %s: %s\n", args[1], strerror(GetLastError()));
        }
    }
#else
    if(args[1]==NULL){
        fprintf(stderr,"sphs: expected argument to \"cd\"\n");
    }
    else{
        if(chdir(args[1])!=0){
            perror("sphs");
        }
    }
#endif
    return 1;
}
int sphs_help(char **args){
    int i;
    printf("Mahos's SPHS\n");
    printf("The following are built in:\n");

    for (i = 0; i < sphs_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }
    printf("This shell can access all the above commands and even all compilers present in System PATH. However certain functionalities are still under development. \n");
    return 1;
}
int sphs_exit(char **args){
    return 0;
}
int sphs_rmdir(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "sphs: expected argument to \"rmdir\"\n");
    } else {
        if (rmdir(args[1]) != 0) {
            perror("sphs");
        }
    }
    return 1;
}
int sphs_rm(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "sphs: expected argument to \"rm\"\n");
    } else {
        if (remove(args[1]) != 0) {
            perror("sphs");
        }
    }
    return 1;
}
int sphs_ls(char **args) {
    DIR *d;
    struct dirent *dir;
    d = opendir(".");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            printf("%s\n", dir->d_name);
        }
        closedir(d);
    } else {
        perror("sphs");
    }
    return 1;
}
int sphs_zeta(char **args) {
    char *zeta1 = "zeta.exe";
    char *zeta_args[] = { zeta1, NULL };
    return sphs_launch(zeta_args);
}
int sphs_path(char **args) {
    char *path = getenv("PATH");
    if (path == NULL) {
        fprintf(stderr, "sphs: could not get PATH\n");
        return 1;
    }
    printf("System PATH:\n%s\n", path);
    return 1;
}

int sphs_exec_path(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "sphs: expected argument to \"exec_path\"\n");
        return 1;
    }
    return sphs_launch(&args[1]);
}

void sphs_Loop(){
    char *line = NULL;
    char **args=NULL;
    int stat=1;
    char cwd[1024];
    printf("\033[1;3;32mMahos's SPHS (a shell in a shell) v1.0. (Please type help and press enter for details).\n");
    do{
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s sphs$ ", cwd);
        } 
        else {
            perror("sphs");
        }
        line=sphs_read_line();
        args=sphs_token_line(line);
        stat=sphs_execute(args);
        free(line);
        free(args);
    }while(stat);
}

int main(int argc, char **argv){
    sphs_Loop();
    return EXIT_SUCCESS;
}
