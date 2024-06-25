#include "smsh.h"
#include <sys/wait.h>
#include <sstream>
#include <fcntl.h>

#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/types.h>


void execute2(char *arglist[]){
    execvp(arglist[0],arglist);
    perror("execute failed");
    exit(1);
}
void execute3(char *arglist[]){
    int pid,exitstatus;

    pid = fork();
    switch (pid) {
    case -1:
        perror("fork failed");
        exit(1);
    case 0:
        execvp(arglist[0],arglist);
        perror("execvp failed");
        exit(1);
    default:
        while (wait(&exitstatus) != pid) {}
        std::cout<<"child exited with status"<<(exitstatus>>8)<<","<<(exitstatus&0377)<<std::endl;
    }
}

int execute4(char *arglist[]){

    std::vector<std::string> args;
    std::string infile, outfile;

    for (int i = 0; arglist[i] != nullptr; ++i) {
        args.push_back(arglist[i]);
    }

    size_t num_commands = 0;
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "|") {
            num_commands++;
        }
    }
    num_commands++;

    size_t cmd_start = 0;
    int prev_fd[2] = {-1, -1};

    for (size_t cmd_index = 0; cmd_index < num_commands; ++cmd_index) {
        std::vector<std::string> cmd_args;
        infile.clear();
        outfile.clear();

        for (; cmd_start < args.size() && args[cmd_start] != "|"; ++cmd_start) {
            if (args[cmd_start] == "<" && cmd_start + 1 < args.size()) {
                infile = args[++cmd_start];
            } else if (args[cmd_start] == ">" && cmd_start + 1 < args.size()) {
                outfile = args[++cmd_start];
            } else {
                cmd_args.push_back(args[cmd_start]);
            }
        }
        cmd_start++; // 跳过管道符号

        if (cmd_args.empty()) continue;

        int fd[2];
        if (cmd_index < num_commands - 1) {
            pipe(fd);
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork failed");
            return 0;
        } else if (pid == 0) {
            signal(SIGINT, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);

            if (!infile.empty()) {
                int in_fd = open(infile.c_str(), O_RDONLY);
                if (in_fd == -1) {
                    perror("open input file failed");
                    exit(1);
                }
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }

            if (cmd_index > 0) { // 不是第一个命令
                dup2(prev_fd[0], STDIN_FILENO);
                close(prev_fd[0]);
                close(prev_fd[1]);
            }

            if (cmd_index < num_commands - 1) { // 不是最后一个命令
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
            }

            if (!outfile.empty()) {
                int out_fd = open(outfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if (out_fd == -1) {
                    perror("open output file failed");
                    exit(1);
                }
                dup2(out_fd, STDOUT_FILENO);
                close(out_fd);
            }

            std::vector<char*> c_args;
            for (auto& arg : cmd_args) {
                c_args.push_back(&arg[0]);
            }
            c_args.push_back(nullptr);

            execvp(c_args[0], c_args.data());
            perror("execvp failed");
            exit(1);
        } else {
            if (cmd_index > 0) {
                close(prev_fd[0]);
                close(prev_fd[1]);
            }

            if (cmd_index < num_commands - 1) {
                prev_fd[0] = fd[0];
                prev_fd[1] = fd[1];
            }

            waitpid(pid, nullptr, 0);
        }
    }

    return 0;
}

int execute5(char *arglist[]){

    int	rv = 0;

    if ( arglist[0] == NULL )
        rv = 0;
    else if ( is_control_command(arglist[0]) )
        rv = control_command(arglist);
    else if ( ok_to_execute() )
        rv = execute4(arglist);
    return rv;
}

