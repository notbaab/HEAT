#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unordered_map>
#include <unordered_set>

#include "console.h"
#include "datastructures/Trie.h"
#include "linenoise/linenoise.h"
#include "str_utils/str_utils.h"
// keep all the command and completions in this space

static std::unordered_set<std::string> commands;
static std::unordered_map<std::string, ConsoleCallback> exitCommands;
static std::unordered_map<std::string, std::vector<std::string>> hints_map;
static DictionaryTrie hintTrie = DictionaryTrie();

void add_command(std::string full_command)
{
    commands.emplace(full_command);

    if (!hintTrie.insert(full_command, 1))
    {
        std::cout << "Failed!" << std::endl;
    }
}

void add_exit_command(std::string full_command, ConsoleCallback callback)
{
    exitCommands.emplace(full_command, callback);
}

// I want to use a ternary search tree for this
void completion(const char* buf, linenoiseCompletions* lc)
{
    std::string s(buf);
    std::vector<std::string> completions = hintTrie.predictCompletions(s, 5);

    for (std::string completion : completions)
    {
        linenoiseAddCompletion(lc, completion.c_str());
    }
}

char* hints(const char* buf, int* color, int* bold)
{
    if (!strcasecmp(buf, "hello"))
    {
        *color = 35;
        *bold = 0;
        return (char*)" World";
    }
    return NULL;
}

void interactive_console(const char* socketPath)
{
    char* line;

    struct sockaddr_un addr;
    char buf[100];
    int fd, rc;

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("socket error");
        exit(-1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socketPath, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    {
        perror("connect error");
        exit(-1);
    }

    std::string getCommands("get-commands\0");

    int sent = send(fd, getCommands.c_str(), getCommands.size(), 0);
    if (sent == -1)
    {
        exit(-1);
    }
    recv(fd, buf, 100, 0);
    std::string str(buf);

    linenoiseSetCompletionCallback(completion);
    linenoiseSetHintsCallback(hints);
    linenoiseHistoryLoad("history.txt"); /* Load the history at startup */

    std::vector<std::string> strings;
    auto commandStrs = str_utils::SplitString(str, "\n");
    for (const auto& command : *commandStrs)
    {
        std::cout << "Adding " << command << std::endl;
        add_command(command);
    }

    while ((line = linenoise("cmd> ")) != NULL)
    {
        /* Do something with the string. */
        if (line[0] != '\0' && line[0] != '/')
        {
            sent = send(fd, line, strlen(line) + 1, 0);
            if (sent == -1)
            {
                std::cout << "Failed sending command" << std::endl;
                continue;
            }

            int bytes = recv(fd, buf, 100, 0);
            std::string str;
            str.assign(buf, bytes);
            std::cout << str << std::endl;

            linenoiseHistoryAdd(line);
            linenoiseHistorySave("history.txt");
        }
        else if (!strncmp(line, "/historylen", 11))
        {
            int len = atoi(line + 11);
            linenoiseHistorySetMaxLen(len);
        }
        else if (line[0] == '/')
        {
            printf("Unrecognized command: %s\n", line);
        }
        free(line);
    }
}

int main(int argc, const char* argv[])
{
    while (argc > 1)
    {
        argc--;
        argv++;
        if (!strcmp(*argv, "--debug-socket"))
        {
            argv++;
            argc--;
            const char* socketPath = *argv;
            if (*socketPath == '\0')
            {
                exit(-1);
            }
            interactive_console(socketPath);
        }
        else
        {
            std::cout << "Usage: " << std::endl;
            std::cout << "        console --debug-socket <socket_location>" << std::endl;
        }
    }
}
