
#include <stdio.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "sh.h"
#include "trie.h"

int main(int argc, char** argv) {
    buildTrie();
    rl_bind_key('\t', auto_complete);

    char histpath[1024];
    const char* home = getenv("HOME");
    if (home)
        snprintf(histpath, sizeof(histpath), "%s/.sh_history", home);
    else
        snprintf(histpath, sizeof(histpath), ".sh_history");

    if (read_history(histpath) == -1) {
        FILE* histfile = fopen(histpath, "w");
        if (histfile) {
            fclose(histfile);
        } else {
            fprintf(stderr, "Warning: Could not create history file at %s\n",
                    histpath);
        }
    }

    sh_loop();

    write_history(histpath);

    return EXIT_SUCCESS;
}
