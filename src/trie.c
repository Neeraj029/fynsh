#include "trie.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>

struct dirent *dir;
TrieNode *root;  // global

int n = 5;
int match_cnt = 0;
char **words = NULL;
char last_match[256];

TrieNode *createNode()
{
    TrieNode *node = (TrieNode *)calloc(1, sizeof(TrieNode));
    for (int i = 0; i < NUM; i++)
        node->children[i] = NULL;
    node->terminal = false;
    return node;
}

int char_index(char c) {
    if (c >= 'a' && c <= 'z') return c - 'a';
    if (c >= '0' && c <= '9') return 26 + (c - '0');
    return -1;
}

void insert(TrieNode *root, const char *word) {
    TrieNode *curr = root;
    while (*word) {
        int idx = char_index(*word);
        if (idx == -1) { word++; continue; }
        if (curr->children[idx] == NULL)
            curr->children[idx] = calloc(1, sizeof(TrieNode));
        curr = curr->children[idx];
        word++;
    }
    curr->terminal = true;
}

void dfs(TrieNode *node, char *prefix, int depth, char word[])
{
    if (node->terminal)
    {
        prefix[depth] = '\0';

        // +1 for null terminator
        int total_len = strlen(word) + strlen(prefix) + 1;
        char *buf = malloc(total_len);
        if (!buf) return;

        strcpy(buf, word);
        strcat(buf, prefix);

        // Grow words array if needed
        if (match_cnt >= n) {
            n += 5;
            char **tmp = realloc(words, n * sizeof(char *));
            if (!tmp) { free(buf); return; }
            words = tmp;
        }

        words[match_cnt] = buf;  // buf already heap-allocated via malloc

        strncpy(last_match, buf, sizeof(last_match) - 1);
        last_match[sizeof(last_match) - 1] = '\0';
        match_cnt++;
    }

    for (int i = 0; i < NUM; i++)
    {
        if (node->children[i] != NULL)
        {
            if (i < 26)
                prefix[depth] = (char)('a' + i);
            else
                prefix[depth] = (char)(i - 26 + '0');
            dfs(node->children[i], prefix, depth + 1, word);
        }
    }
}

int searchWord(TrieNode *root, char *word)
{
    // // Free previous results
    // if (words) {
    //     for (int i = 0; i < match_cnt; i++)
    //         free(words[i]);
    //     free(words);
    //     words = NULL;
    // }

    // Reset globals every call
    match_cnt = 0;
    n = 5;

    words = malloc(n * sizeof(char *));
    if (!words) return 0;

    if (!root) return 0;

    TrieNode *temp = root;
    for (int i = 0; i < (int)strlen(word); i++)
    {
        int idx = char_index(word[i]);
        if (idx == -1) continue;
        if (temp->children[idx] == NULL)
            return 0;
        temp = temp->children[idx];
    }

    char prefix[512];
    memset(prefix, 0, sizeof(prefix));

    dfs(temp, prefix, 0, word);

    return match_cnt;
}

void buildTrie()
{
    root = createNode();
    char *septr = ":";
    char *path_var = getenv("PATH");
    if (!path_var) return;

    char *path_cp = strdup(path_var);
    if (!path_cp) return;

    char *tkn = strtok(path_cp, septr);
    DIR *d;

    while (tkn)
    {
        if (strncmp(tkn, "/mnt/", 5) != 0)
        {
            d = opendir(tkn);
            if (d)
            {
                while ((dir = readdir(d)) != NULL)
                {
                    // Skip . and ..
                    if (strcmp(dir->d_name, ".") == 0 ||
                        strcmp(dir->d_name, "..") == 0)
                        continue;
                    insert(root, dir->d_name);
                }
                closedir(d);
            }
        }
        tkn = strtok(NULL, septr);
    }

    free(path_cp);
}