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

#define DEBUG_MODE 1

int MAX_CMD_LEN = 1024;
int MAX_HIST_SIZE = 10;
int MAX_ARGS = 100;
char W_DIR[1024]; //working directory

int execute(char *args[], int background = 0) {
    int pid;

    pid = fork();
    if(pid == -1) {
        
        fprintf(stderr, "Error in fork.\n");
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
    int i = 0;
    char *args[MAX_ARGS];

    while(tokenize >> token) {
        if(i > MAX_ARGS) {
            fprintf(stderr, "shell: Too many arguments.\n");
            return 1;
        }
        args[i] = new char[token.length() + 1]; //yay, memory management :(
        strcpy(args[i], token.c_str());
        i += 1;
    }
    args[i] = NULL;

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
    
    execute(args);

    for (int j = 0; j < i; j += 1) delete [] args[j]; //yay, memory management :(

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

    getcwd(W_DIR, 1024);

    if(DEBUG_MODE) cout << "Initialized successfully\n";

    while (true) {
        string cmd;
        cout << W_DIR << " $ "; 
        getline(cin, cmd);
        if(cmd == "exit") break; //todo, make this use a cmp instead of jank
        process_cmd(cmd);
    }

    return 0;
}