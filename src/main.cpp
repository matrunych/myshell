#include <iostream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <readline/readline.h>
#include <readline/history.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

int error = 0;

int end_with(std::string str, std::string substr) {
    if (str.size() >= substr.size() &&
        str.compare(str.size() - substr.size(), substr.size(), substr) == 0)
        return 1;
    else
        return 0;
}

void mexport(std::vector <std::string> argv) {
    std::string name_val = argv.at(1);

    if (name_val.find('=') == std::string::npos) {
        error = 1;
        return;
    }

    std::string name = name_val.substr(0, name_val.find('='));
    std::string val = name_val.substr(name_val.find('=') + 1, name_val.size() - name_val.find('='));

    setenv(name.c_str(), val.c_str(), 1);

    error = 0;
}

void mecho(std::vector <std::string> argv) {
    for (int i = 1; i < argv.size(); i++) {
        if (argv.at(i).at(0) == '$') {
            char* env_var = getenv(argv[i].substr(1, argv[i].size() - 1).c_str());

            if (env_var != nullptr) {
                std::cout << env_var << " ";
            }
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

bool is_number(const std::string &s) {
    return !s.empty() && std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

void mexit(int argc, std::vector <std::string> argv) {
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

int fork_exec(char **args) {
    pid_t pid = fork();
    if (pid == -1) {
        return -1;
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        execvp(args[0], args);
        _exit(EXIT_FAILURE);
    }
    return 1;
}


void execute_script(std::string file) {
    std::ifstream script(file);
    if (!script.is_open()) {
        std::cout << "Cannot open file" << std::endl;
        return;
    }
    std::string line;

    while (std::getline(script, line)) {
        std::vector <std::string> argv;
        std::istringstream ss(line);
        std::string word;
        while (ss >> word) {
            if (word[0] == '#') {
                break;
            }
            argv.push_back(word);
        }

        std::vector<char *> args;
        for (const auto &arg : argv)
            args.push_back((char *) arg.data());
        args.push_back(nullptr);
        char **argss = &args[0];

        fork_exec(argss);
    }
}


int main(int argc, char **argv) {
    auto p = getenv("PATH");
    std::string path = p;
    path += ":.";
    setenv("PATH", path.c_str(), 1);

    if (argc == 2) {
        execute_script(argv[1]);
        return 0;
    }
    if (argc > 2) {
        std::cout << "Invalid number of arguments" << std::endl;
        return -1;
    }


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
        std::vector <std::string> ret;

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

        namespace po = boost::program_options;

        po::options_description visible("Supported options");
        visible.add_options()
                ("help,h", "Print help message.");

        po::variables_map vm;
        po::store(po::command_line_parser(argc, args).
                options(visible).run(), vm);
        po::notify(vm);


        if ((first.rfind("./", 0) == 0) && end_with(first, ".msh")) {
            std::vector<char *> pr;
            pr.push_back((char *) "myshell");

            pr.push_back((char *) first.substr(2, first.length() - 2).c_str());

            fork_exec(&pr[0]);
        }
        else if ((first == ".") && end_with(ret.at(1), ".msh")) {
            execute_script(ret.at(1));
        }
        else if (first == "mexport") {
            if (vm.count("help")) {
                std::cout << "Create new environment variable (var_name=VAL).\n" << visible << std::endl;
            } else {
                mexport(ret);
            }
        }
        else if (first == "mecho") {
            if (vm.count("help")) {
                std::cout << "Print given variables (starting with $) or text into console.\n" << visible << std::endl;
            } else {
                mecho(ret);
            }
        }
        else if (first == "merrno") {
            if (vm.count("help")) {
                std::cout << "Print exit code of last command into console.\n" << visible << std::endl;
            } else {
                merrno();
            }
        }
        else if (first == "mpwd") {
            if (vm.count("help")) {
                std::cout << "Print current path.\n" << visible << std::endl;
            } else {
                mpwd();
            }
        }
        else if (first == "mcd") {
            if (vm.count("help")) {
                std::cout << "Change working directory.\n" << visible << std::endl;
            } else {
                mcd(argv, d);
            }
        }
        else if (first == "mexit") {
            if (vm.count("help")) {
                std::cout << "Exit myshell with given exit code.\n" << visible << std::endl;
            } else {
                mexit(argc, ret);
            }
        }
        else {
            fork_exec(args);
        }

        new_buf.clear();
    }

    return 0;
}
