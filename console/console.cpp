#include <iostream>
#include <sstream>
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

// The hint map is a mapping of <command> <potential-values>
// TODO: I should add a way to traverse the trie with no key, so we don't need to
// have the flat map as well.
static std::unordered_map<std::string, DictionaryTrie> hints_map;
static std::unordered_map<std::string, std::vector<std::string>> flatHintMap;
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
    std::istringstream iss(buf);

    // Predicts the first command completion
    std::vector<std::string> completions = hintTrie.predictCompletions(iss.str(), 5);

    for (std::string completion : completions)
    {
        linenoiseAddCompletion(lc, completion.c_str());
    }

    auto stringParts = str_utils::SplitString(buf, " ");

    if (stringParts.size() >= 1)
    {
        if (hints_map.count(stringParts[0]) == 0)
        {
            return;
        }

        if (stringParts.size() == 1)
        {
            for (std::string completion : flatHintMap.at(stringParts[0]))
            {
                std::string fullCompletion(stringParts[0]);
                fullCompletion += " " + completion;
                linenoiseAddCompletion(lc, fullCompletion.c_str());
            }
        }
        else
        {
            auto hintCompletions = hints_map.at(stringParts[0]).predictCompletions(stringParts[1], 100);
            for (std::string completion : hintCompletions)
            {
                std::string fullCompletion(stringParts[0]);
                fullCompletion += " " + completion;
                linenoiseAddCompletion(lc, fullCompletion.c_str());
            }
        }
    }
}

void add_hint(std::string commandToHint, std::string valueToHint)
{
    if (hints_map.count(commandToHint) == 0)
    {
        hints_map.emplace(commandToHint, DictionaryTrie());
        flatHintMap.emplace(commandToHint, 1);
    }

    hints_map.at(commandToHint).insert(valueToHint, 1);
    flatHintMap.at(commandToHint).emplace_back(valueToHint);
}

char* hints(const char* buf, int* color, int* bold)
{
    // TODO: Add actual hints
    // if (!strcasecmp(buf, "set-var"))
    // {
    //     *color = 35;
    //     *bold = 0;
    //     return (char*)" stuff";
    // }
    return NULL;
}

void interactive_console(const char* socketPath)
{
    char* line;

    struct sockaddr_un addr;
    char buf[100];
    int fd;

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

    auto commandStrs = str_utils::SplitString(str, "\n");
    for (const auto& command : commandStrs)
    {
        add_command(command);
    }

    std::string getCompletions("get-command-hints\0");

    sent = send(fd, getCompletions.c_str(), getCompletions.size(), 0);
    if (sent == -1)
    {
        exit(-1);
    }
    recv(fd, buf, 100, 0);
    std::string completions(buf);

    auto hintStrs = str_utils::SplitString(completions, "\n");
    for (const auto& command : hintStrs)
    {
        auto hints = str_utils::SplitString(command, " ");
        if (hints.size() > 1)
        {
            for (unsigned i = 1; i < hints.size(); ++i)
            {
                add_hint(hints.at(0), hints.at(i));
            }
        }
    }

    while ((line = linenoise("cmd> ")) != NULL)
    {
        // Do something with the string.
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
