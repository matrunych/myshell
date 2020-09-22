#include <iostream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include "operations/operations.hpp"
#include <readline/readline.h>
#include <readline/history.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

int launch(char **args) {
    pid_t parent = getpid();
    pid_t pid = fork();
    if (pid == -1) {
        return -1;
    }
    else if (pid > 0) {
// We are in parent process
        int status;
        waitpid(pid, &status, 0);
    } else {
        char *env[] = { "HOME=/usr/home", "LOGNAME=home", (char *)0 };
//        execve(args[0], args, env);
        execvp(args[0], args);
        _exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv) {
//    if (argc > 1 && std::string(argv[1]) == "-d") {
//        rl_bind_key('\t', rl_insert);
//    }

//    for (char **ep = environ; *ep != NULL; ep++) {
//        puts(*ep);
//    }

    char* buf;
    boost::filesystem::path full_path(boost::filesystem::current_path());
//    std::cout << "Current path is : " << full_path << std::endl;
//    std::cout << full_path.string() + "$ ";
    std :: string d = full_path.string() + " $ ";

    while ((buf = readline(d.c_str())) != nullptr) {
        if (strlen(buf) > 0) {
            add_history(buf);
        }

//        printf("[%s]\n", buf);
        std::istringstream ss (buf);
        std::vector<std::string> ret;

        std::copy(std::istream_iterator<std::string>(ss),
                  std::istream_iterator<std::string>(),
                  std::back_inserter(ret));

//        for(auto x: ret){
//            std::cout<<x<<std::endl;
//        }

        std::vector<char*> argv;
        for (const auto& arg : ret)
            argv.push_back((char*)arg.data());
        argv.push_back(nullptr);

        char** args = &argv[0];


        std::string first = ret.at(0);
        char fch = first.at(0);

        if (fch == '/') {
            launch(args);
        }

        if (first == "ls") {
            launch(args);
        }

        if (first == "mexport") {
            std::string name_val = ret.at(1);
            std::string name = name_val.substr(0, name_val.find('='));
            std::string val = name_val.substr(name_val.find('=') + 1, name_val.size() - name_val.find('='));
//            std::cout << name << " " << val <<std::endl;
            setenv(name.c_str(), val.c_str(), 1);

//            for (char **ep = environ; *ep != NULL; ep++) {
//                puts(*ep);
//            }
        }

        if (first == "mecho" && ret.at(1).at(0) == '$') {
            std::cout << getenv(ret[1].substr(1, ret[1].size() - 1).c_str()) << std::endl;
        }

        free(buf);
    }

//    std::string str(buf);

//    std::istringstream ss (buf);
//    std::vector<std::string> ret;
//
//    std::copy(std::istream_iterator<std::string>(ss),
//              std::istream_iterator<std::string>(),
//              std::back_inserter(ret));
//
//    for(auto x: ret){
//        std::cout<<x<<std::endl;
//    }
    return 0;





    return EXIT_SUCCESS;

}
