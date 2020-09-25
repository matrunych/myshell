#include <iostream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include "operations/operations.hpp"
#include <readline/readline.h>
#include <readline/history.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

int error = 0;

void mexport(std::vector<std::string> argv) {
    std::string name_val = argv.at(1);

    if (name_val.find('=') == std::string::npos) {
        error = 1;
        return;
    }

    std::string name = name_val.substr(0, name_val.find('='));
    std::string val = name_val.substr(name_val.find('=') + 1, name_val.size() - name_val.find('='));

    setenv(name.c_str(), val.c_str(), 1);

    error = 0;

//            for (char **ep = environ; *ep != NULL; ep++) {
//                puts(*ep);
//            }
}

void mecho(std::vector<std::string> argv) {
    for (int i = 1; i < argv.size(); i++) {
        if (argv.at(i).at(0) == '$') {
            std::cout << getenv(argv[i].substr(1, argv[i].size() - 1).c_str()) << " ";
        } else {
            std::cout << argv[i] << " ";
        }
    }
    std::cout << std::endl;

    error = 0;
}

void merrno() {
    std::cout << error << std::endl;
}

void mpwd() {
    std::cout << boost::filesystem::current_path().string() << std::endl;

    error = 0;
}

void mcd(std::vector<char *> argv, std::string &d) {
    error = chdir(argv[1]);

    d = boost::filesystem::current_path().string() + " $ ";
}

bool is_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

void mexit(int argc, std::vector<std::string> argv) {
    if (argc == 1) {
        _exit(0);
    } else {
        if (is_number(argv[1])) {
            _exit(std::stoi(argv[1]));
        } else {
            error = 1;
        }
    }
}

int launch(char **args) {
    pid_t parent = getpid();
    pid_t pid = fork();
    if (pid == -1) {
        return -1;
    } else if (pid > 0) {
// We are in parent process
        int status;
        waitpid(pid, &status, 0);
    } else {
//        char *env[] = {"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin", "HOME=/usr/home", "LOGNAME=home", (char *) 0};
//        execve(args[0], args, environ);
        execvp(args[0], args);
        _exit(EXIT_FAILURE);
    }
    return 1;
}

int main(int argc, char **argv) {
//    if (argc > 1 && std::string(argv[1]) == "-d") {
//        rl_bind_key('\t', rl_insert);
//    }

//    for (char **ep = environ; *ep != NULL; ep++) {
//        puts(*ep);
//    }



    char *buf;
    std::string new_buf = "";
    boost::filesystem::path full_path(boost::filesystem::current_path());
    std::string d = full_path.string() + " $ ";

    while ((buf = readline(d.c_str())) != nullptr) {
        if (strlen(buf) > 0) {
            add_history(buf);
        }

        for (int i = 0; i < strlen(buf); i++) {
            if (buf[i] != '#') {
                new_buf += buf[i];
            } else { break; }
        }
        free(buf);


        std::istringstream ss(new_buf);
        std::vector<std::string> ret;

        std::copy(std::istream_iterator<std::string>(ss),
                  std::istream_iterator<std::string>(),
                  std::back_inserter(ret));


        std::vector<char *> argv;
        for (const auto &arg : ret)
            argv.push_back((char *) arg.data());
        argv.push_back(nullptr);

        char **args = &argv[0];

        int argc = ret.size();

        std::string first = ret.at(0);
        char fch = first.at(0);

        namespace po = boost::program_options;

        po::options_description visible("Supported options");
        visible.add_options()
                ("help,h", "Print help message.");

        po::variables_map vm;
        po::store(po::command_line_parser(argc, args).
                options(visible).run(), vm);
        po::notify(vm);

//        if (fch == '/') {
//            launch(args);
//        }
//
//        if (first == "ls") {
//            launch(args);
//        }

        if (first == "mexport") {
            if (vm.count("help")) {
                std::cout << "Create new environment variable (var_name=VAL).\n" << visible << std::endl;
            } else {
                mexport(ret);
            }
        }

        if (first == "mecho") {
            if (vm.count("help")) {
                std::cout << "Print given variables (starting with $) or text into console.\n" << visible << std::endl;
            } else {
                mecho(ret);
            }
        }

        if (first == "merrno") {
            if (vm.count("help")) {
                std::cout << "Print exit code of last command into console.\n" << visible << std::endl;
            } else {
                merrno();
            }
        }

        if (first == "mpwd"){
            if (vm.count("help")) {
                std::cout << "Print current path.\n" << visible << std::endl;
            } else {
                mpwd();
            }
        }

        if (first == "mcd"){
            if (vm.count("help")) {
                std::cout << "Change working directory.\n" << visible << std::endl;
            } else {
                mcd(argv, d);
            }
        }

        if (first == "mexit") {
            if (vm.count("help")) {
                std::cout << "Exit myshell with given exit code.\n" << visible << std::endl;
            } else {
                mexit(argc, ret);
            }
        }

        launch(args);



        new_buf.clear();

    }


    return 0;


    return EXIT_SUCCESS;

}
