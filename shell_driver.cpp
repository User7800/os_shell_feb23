#include <iostream>
#include <getopt.h>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <sstream>

using std::vector;
using std::string;
using std::stringstream;
using std::cout;
using std::cin;
using std::getline;

#define DEBUG_MODE 1

int MAX_CMD_LEN = 1024;
int MAX_HIST_SIZE = 10;
int MAX_ARGS = 100;
string W_DIR; //working directory

int update_wdir() {
    char buf[1024];
    getcwd(buf, 1024);
    W_DIR = buf;
}

int execute(string cmd, int background = 0) {
    stringstream tokenize(cmd);
    string token;
    int pid, i;
    char *args[MAX_ARGS];

    i = 0;
    while(tokenize >> token) {
        //todo: add check for MAX_ARGS here
        args[i] = new char[token.length() + 1]; //yay, memory management :-(
        strcpy(args[i], token.c_str());
        i += 1;
    }
    args[i] = NULL;

    pid = fork();
    if(pid == -1) {
        fprintf(stderr, "Error in fork.\n");
        abort();
    } else if(pid != 0) {
        if (!background) waitpid(pid, NULL, 0);
    } else {
        execvp(args[0], args); //would usually remove the {} but that would be kinda arcane looking here
    }

    for (int j = 0; j < i; j += 1) delete [] args[j]; //yay, memory management :-(

    return 0;

}

int flow(string cmd) {
    if(cmd.find("cd") != string::npos) {
        char *args[MAX_ARGS];

    }
    execute(cmd);
    return 0;

}

int main(int argc, char* argv[]) {

    int opt;
    while ((opt = getopt(argc, argv, "h:l:a:")) != -1) {
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
            case '?':
                if (optopt == 'h' || optopt == 's') {
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                } 
                return 1; //dont use abort here because comon user-error, core dump not useful
            default:
                abort();
        }
    }

    update_wdir();

    if(DEBUG_MODE) cout << "Initialized successfully\n";

    while (true) {
        string cmd;
        cout << W_DIR << " $ "; 
        getline(cin, cmd);
        if(cmd == "exit") break; //todo, make this use a cmp instead of jank
        execute(cmd);
    }

    return 0;
}