#include <iostream>
#include <getopt.h>
#include <string>
#include <vector>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/wait.h>
#include <sstream>
#include <fstream>

using std::vector;
using std::string;
using std::stringstream;
using std::ofstream;
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

int execute(char *args[], int background, int redirect, string filename);
int process_cmd(string cmd);
void audit(int signal);

int execute(char *args[], int background = 0, int redirect = 0, string filename = "") {
    int pid;

    pid = fork();
    if(pid == -1) {
        perror("fork");
        return 1;
    } else if(pid != 0) {
        if (!background) waitpid(pid, NULL, 0);
        else cout << "Process " << pid << " running in background.\n";
    } else {
        if(redirect == 1)
            freopen(filename.c_str(), "w", stdout);
        if(redirect == 2) 
            freopen(filename.c_str(), "r", stdin); //who needs to clean when you have a sacrificial fork()
        execvp(args[0], args); //would usually remove the {} but that would be kinda arcane looking here
    }
    return 0;

}

int process_cmd(string cmd) {
    stringstream tokenize(cmd);
    string token, filename;
    unsigned int i = 0, background = 0, redirect = 0, j, return_val;
    char *args[MAX_ARGS];

    history.push_back(cmd);
    if(history.size() > MAX_HIST_SIZE) {
        history.erase(history.begin());
    }

    while(tokenize >> token) {
        if(i > MAX_ARGS) {
            fprintf(stderr, "shell: Too many arguments.\n");
            return_val = 1;
            goto END;
        }
        if(token == "&") {
            background = 1;
            continue;
        }
        if(token == ">") {
            tokenize >> token;
            filename = token;
            redirect = 1;
            continue;
        }
        if(token == "<") {
            tokenize >> token;
            filename = token;
            redirect = 2;
            continue;
        }
        args[i] = new char[token.length() + 1]; //yay, memory management :(
        strcpy(args[i], token.c_str());
        i += 1;
    }
    args[i] = NULL;

    if(args[0] == NULL) {
        return_val = 0;
        goto END;
    } 

    if(strcmp(args[0], "exit") == 0) exit(EXIT_SUCCESS);

    if(strcmp(args[0], "audit") == 0) audit(0);

    if(strcmp(args[0], "cd") == 0) {
        if(i > 2) {
            fprintf(stderr, "cd: Too many arguments.\n");
            return_val = 1;
            goto END;
        }
        if(i == 1) {
            return_val = 0;
            goto END;
        } //cd into current dir, bash would go to ~ here

        if(chdir(args[1])) {
            perror("cd");
            return_val = 1;
            goto END;
        }
        getcwd(W_DIR, 1024);
        return_val = 0;
        goto END;
    }

    if(strcmp(args[0], "history") == 0) {
        for(i = 0; i < history.size(); i += 1) {
            cout << history[i] << '\n';
        }
        return_val = 0;
        goto END;
    }

    if(strcmp(args[0], "env") == 0) {
        cout << "MAX_CMD_LEN = " << MAX_CMD_LEN << '\n';
        cout << "MAX_HIST_SIZE = " << MAX_HIST_SIZE << '\n';
        cout << "MAX_ARGS = " << MAX_ARGS << '\n';
        cout << "W_DIR = " << W_DIR << '\n';
        cout << "SHELL_CHAR = " << SHELL_CHAR << '\n';
        return_val = 0;
        goto END;
    }

    execute(args, background, redirect, filename);

    END:

    for (j = 0; j < i; j += 1) delete [] args[j]; //yay, memory management :(

    return return_val;

}

void audit(int signal) {
    ofstream audit_log("audit.log");
    for(unsigned int i = 0; i < history.size(); i += 1) {
        audit_log << history[i] << '\n';
    }
    audit_log.close();
    exit(EXIT_SUCCESS);
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
                return 1;
            default:
                abort();
        }
    }

    getcwd(W_DIR, 1024);
    signal(SIGUSR1, audit);

    if(DEBUG_MODE) cout << "std_out = " << STDOUT_FILENO << '\n';
    if(DEBUG_MODE) cout << "std_in = " << STDIN_FILENO << '\n';
    if(DEBUG_MODE) cout << "Initialized successfully\n";

    string cmd;
    while (true) {
        cout << W_DIR << ' ' << SHELL_CHAR << ' '; 
        getline(cin, cmd);
        process_cmd(cmd);
    }

    return 0;
}