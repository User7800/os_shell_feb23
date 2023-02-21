#include <iostream>
#include <getopt.h>
#include <string>
#include <vector>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/wait.h>
#include <sstream>

using std::vector;
using std::string;
using std::stringstream;
using std::cout;
using std::cin;
using std::getline;

#define DEBUG_MODE 0

unsigned int MAX_CMD_LEN = 1024;
unsigned int MAX_HIST_SIZE = 10;
unsigned int MAX_ARGS = 100;
char W_DIR[1024]; //working directory
char SHELL_CHAR = '$';
vector<string> history;

int execute(char *args[], int background = 0) {
    int pid;

    pid = fork();
    if(pid == -1) {
        perror("fork");
        return 1;
    } else if(pid != 0) {
        if (!background) waitpid(pid, NULL, 0);
    } else {
        execvp(args[0], args); //would usually remove the {} but that would be kinda arcane looking here
    }
    return 0;

}

int process_cmd(string cmd) {
    stringstream tokenize(cmd);
    string token;
    unsigned int i = 0, background = 0, j;
    char *args[MAX_ARGS];

    history.push_back(cmd);
    if(history.size() > MAX_HIST_SIZE) {
        history.erase(history.begin());
    }

    while(tokenize >> token) {
        if(i > MAX_ARGS) {
            fprintf(stderr, "shell: Too many arguments.\n");
            return 1;
        }
        if(token == "&") {
            background = 1;
            continue;
        }
        args[i] = new char[token.length() + 1]; //yay, memory management :(
        strcpy(args[i], token.c_str());
        i += 1;
    }
    args[i] = NULL;

    if(args[0] == NULL) return 0;

    if(strcmp(args[0], "cd") == 0) {
        if(i > 2) {
            fprintf(stderr, "cd: Too many arguments.\n");
            return 1;
        }
        if(i == 1) return 0; //cd into current dir, bash would go to ~ here

        if(chdir(args[1])) {
            perror("cd");
            return 1;
        }
        getcwd(W_DIR, 1024);
        return 0;
    }

    if(strcmp(args[0], "history") == 0) {
        for(i = 0; i < history.size(); i += 1) {
            cout << history[i] << '\n';
        }
        return 0;
    }

    if(strcmp(args[0], "env") == 0) {
        cout << "MAX_CMD_LEN = " << MAX_CMD_LEN << '\n';
        cout << "MAX_HIST_SIZE = " << MAX_HIST_SIZE << '\n';
        cout << "MAX_ARGS = " << MAX_ARGS << '\n';
        cout << "W_DIR = " << W_DIR << '\n';
        cout << "SHELL_CHAR = " << SHELL_CHAR << '\n';
        return 0;
    }

    execute(args, background);

    for (j = 0; j < i; j += 1) delete [] args[j]; //yay, memory management :(

    return 0;

}

int main(int argc, char* argv[]) {

    int opt;
    while ((opt = getopt(argc, argv, "h:l:a:c:")) != -1) {
        switch (opt) {

            case 'h':
                MAX_HIST_SIZE = atoi(optarg);
                if(DEBUG_MODE) cout << "Option -h given with argument " << optarg << '\n';
                break;
            case 'l':
                MAX_CMD_LEN = atoi(optarg);
                if(DEBUG_MODE) cout << "Option -l given with argument " << optarg << '\n';
                break;
            case 'a':
                MAX_ARGS = atoi(optarg);
                if(DEBUG_MODE) cout << "Option -a given with argument " << optarg << '\n';
                break;
            case 'c':
                SHELL_CHAR = optarg[0];
                if(DEBUG_MODE) cout << "Option -c given with argument " << optarg << '\n';
                break;
            case '?':
                if (optopt == 'h' || optopt == 's') {
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                } 
                return 1; //dont use abort here because comon user-error, core dump not useful
            default:
                abort();
        }
    }

    getcwd(W_DIR, 1024);

    if(DEBUG_MODE) cout << "Initialized successfully\n";

    while (true) {
        string cmd;
        cout << W_DIR << ' ' << SHELL_CHAR << ' '; 
        getline(cin, cmd);
        if(cmd == "exit") break; //todo, make this use a cmp instead of jank
        process_cmd(cmd);
    }

    return 0;
}