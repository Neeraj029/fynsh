#include "sh.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "trie.h"
#define RESET "\033[0m"
#define GREEN "\033[34m"
#include <readline/history.h>
#include <readline/readline.h>
#define PATH_MAX 70
#include <pwd.h>

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {"cd", "help", "exit"};

int (*builtin_func[])(char **) = {&sh_cd, &sh_help, &sh_exit};

int sh_num_builtins() { return sizeof(builtin_str) / sizeof(char *); }

/*
  Builtin function implementations.
*/

int sh_cd(char **args)
{
    // Use HOME if arg is "~", otherwise use the argument provided
    char *path = (args[1] == NULL || strcmp(args[1], "~") == 0) ? getenv("HOME")
                                                                : args[1];
    if (chdir(path) != 0)
    {
        perror("sh");
    }
    return 1;
}

int sh_help(char **args)
{
    int i;
    printf("Fenric's simple shell\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (i = 0; i < sh_num_builtins(); i++)
    {
        printf("  %s\n", builtin_str[i]);
    }

    return 1;
}

void print_space(char *fname)
{
    int char_limit = 28;
    int sizeoff_name = strlen(fname);
    printf(fname);
    // printf("%d", sizeoff_name);
    if (sizeoff_name <= char_limit)
    {
        // printf("-%d", char_limit - sizeoff_name);
        for (int i = 0; i < (char_limit - sizeoff_name); i++)
        {
            putchar(' ');
        }
    }
    else
    {
        printf("\t");
    }
}

int split_by(char* command, char* dividend, char *cmds[10][10]){
        char *tkn = strtok(command, " ");
    int cmd_cnt = 0, arg_cnt = 0;

    while (tkn != NULL)
    {
        if (strcmp(tkn, dividend) == 0)
        {
            cmds[cmd_cnt][arg_cnt] = NULL;
            cmd_cnt++;
            arg_cnt = 0;
        }
        else
        {
            cmds[cmd_cnt][arg_cnt++] = tkn;
        }
        tkn = strtok(NULL, " ");
    }
    cmds[cmd_cnt][arg_cnt] = NULL;

    int n = cmd_cnt + 1;
    return n;
} 

int fork_n_wait(char *args[]){
    pid_t pid;
    pid = fork();
    if(pid==0){
        execvp(args[0],args);
        perror(args[0]);
        exit(1);
    }

    int status;
    waitpid(pid, &status, 0);

    // Return 0 on success, non-zero on failure
    return !(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}   

int and_cmd_launch(char command[]){
    char *cmds[10][10];
    int n = split_by(command,"&&",cmds);
    for(int i = 0;i<n;i++){
        if(fork_n_wait(cmds[i])){
            printf("%s failed. exiting\n", cmds[i][0]);
            return 1;
        }
    }

}
void pipe_cmd_launch(char command[])
{
    char *cmds[10][10];
    int n = split_by(command,"|", cmds);

    int (*pids)[2] = malloc((n - 1) * sizeof(int[2]));
    for (int i = 0; i < n - 1; i++)
        pipe(pids[i]);

    for (int i = 0; i < n; i++)
    {
        if (fork() == 0)
        {
            if (i > 0)
                dup2(pids[i - 1][0], STDIN_FILENO);
            if (i < n - 1)
                dup2(pids[i][1], STDOUT_FILENO);

            for (int j = 0; j < n - 1; j++)
            {
                close(pids[j][0]);
                close(pids[j][1]);
            }
            execvp(cmds[i][0], cmds[i]);
            perror(cmds[i][0]);
            exit(1);
        }
    }

    for (int i = 0; i < n - 1; i++)
    {
        close(pids[i][0]);
        close(pids[i][1]);
    }
    for (int i = 0; i < n; i++)
        wait(NULL);

    free(pids);
}

int sh_exit(char **args) { return 0; }

int auto_complete(int count, int key)
{
    if (root == NULL)
        return 0;

    char *last = strrchr(rl_line_buffer, ' ');
    char *buf = last ? last + 1 : rl_line_buffer;

    if (!buf[0]) // 1st case (nothing)
    {
        return 0;
    }
    else if (!last && buf[0]) // 2nd case (command autocomplete)
    {
        printf("\n");
        int matches = searchWord(root, buf);
        if (matches == 1)
        {
            // only one match — autocomplete it
            rl_replace_line(last_match, 0);
            rl_point = rl_end; // move cursor to end
        }
        int name_per_row = 4;
        for (int i = 0; i < matches; i++)
        {
            if (i > 0 && i % name_per_row == 0)
                printf("\n");
            print_space(words[i]);
        }
        printf("\n");
        for (int i = 0; i < match_cnt; i++)
        {
            free(words[i]);
        }
        free(words);
    }

    else
    { // 3rd case (file autocomplete)
        char *aftr;
        char bfr[100] = {0};
        DIR *dir = NULL;
        char *slash_last = strrchr(buf, '/');
        if (slash_last)
        {
            aftr = slash_last + 1;
            int len = slash_last - buf + 1;
            strncpy(bfr, buf, len);
            dir = opendir(bfr);
        }
        else
        {
            dir = opendir(".");
        }

        char *prefix = slash_last ? aftr : buf;
        // DIR* dir = opendir(path);
        if (!dir)
        {
            printf("Path: %s", bfr);
            perror("opendir");
            return 1;
        }

        int cntr = 0;
        int matches = 0;
        char first[256] = {0};
        struct dirent *entry;

        while ((entry = readdir(dir)) != NULL)
        {
            if (strcmp(entry->d_name, ".") == 0 ||
                strcmp(entry->d_name, "..") == 0)
                continue;

            if (strncmp(entry->d_name, prefix, strlen(prefix)) == 0)
            {
                matches++;
                if (matches == 1)
                {
                    strncpy(first, entry->d_name, sizeof(first) - 1);
                }
                else
                {
                    if (matches == 2)
                    {
                        // first match was held, print it now
                        printf("\n");
                        print_space(first);
                        cntr++;
                    }
                    if (cntr % 4 == 0)
                        printf("\n");
                    print_space(entry->d_name);
                    cntr++;
                }
            }
        }

        if (matches == 1)
        {
            char completed[1024] = {0};

            char completed_slash[2024] = {0};
            if (slash_last)
            {
                strcpy(completed_slash, bfr);
                strcat(completed_slash, first);
                strcpy(first, completed_slash);
            }
            int prefix_len = buf - rl_line_buffer;
            strncpy(completed, rl_line_buffer, prefix_len);
            strncat(completed, first, sizeof(completed) - prefix_len - 1);
            // printf("|%s|\n", bfr);
            // printf("|%s|\n", completed);

            rl_replace_line(completed, 0);
            rl_point = rl_end;
            rl_redisplay();
            return 0; // return early, skip the rl_on_new_line/rl_redisplay at
                      // the bottom
        }
        else if (matches > 1)
        {
            printf("\n");
        }
        else if (matches == 0)
        {
            return 0;
        }

        closedir(dir);
    }

    rl_on_new_line(); // tell readline cursor is on a new line
    rl_redisplay();   // redraw prompt
    return 0;
}

int sh_launch(char **args)
{
    pid_t pid;
    int status;
    if (strrchr(rl_line_buffer, '|'))
    {
        pipe_cmd_launch(rl_line_buffer);
        return 1;
    }else if(strstr(rl_line_buffer, "&&")){
        and_cmd_launch(rl_line_buffer);
        return 1;
    }
    else
    {

        pid = fork();
        if (pid == 0)
        {
            // Child process
            if (execvp(args[0], args) == -1)
            {
                perror("sh");
            }
            exit(EXIT_FAILURE);
        }
        else if (pid < 0)
        {
            // Error forking
            perror("sh");
        }
        else
        {
            // Parent process
            do
            {
                waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }

        return 1;
    }
}

int sh_execute(char **args)
{
    int i;

    if (args[0] == NULL)
    {
        // An empty command was entered.
        return 1;
    }

    for (i = 0; i < sh_num_builtins(); i++)
    {
        if (strcmp(args[0], builtin_str[i]) == 0)
        {
            return (*builtin_func[i])(args);
        }
    }

    return sh_launch(args);
}

#define sh_TOK_BUFSIZE 64
#define sh_TOK_DELIM " \t\r\n\a"

char **sh_split_line(char *line)
{
    int bufsize = sh_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token, **tokens_backup;

    if (!tokens)
    {
        fprintf(stderr, "sh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, sh_TOK_DELIM);
    while (token != NULL)
    {
        tokens[position] = token;
        position++;

        if (position >= bufsize)
        {
            bufsize += sh_TOK_BUFSIZE;
            tokens_backup = tokens;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens)
            {
                free(tokens_backup);
                fprintf(stderr, "sh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, sh_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

/**
   @brief Loop getting input and executing it.
 */
char cwd[PATH_MAX];
char prompt[PATH_MAX + 20];

void sh_loop(void)
{
    rl_initialize();

    char **args;
    int status;
    struct passwd *pw = getpwuid(getuid());
    char *name = pw->pw_name;
    char *buf;
    do
    {
        getcwd(cwd, sizeof(cwd));
        snprintf(prompt, sizeof(prompt),
                 "\001" GREEN "\002%s\001" RESET "\002:\001" GREEN
                 "\002%s\001" RESET "\002$ ",
                 name, cwd);
        buf = readline(prompt);
        if (strncmp(buf, "cd", 3) == 0)
        {
            chdir(buf + 3);
        }
        if (strlen(buf) > 0)
        {
            add_history(buf);
        }
        args = sh_split_line(buf);

        status = sh_execute(args);

        free(buf);
        free(args);
    } while (status && buf);
}
