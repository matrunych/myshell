#include <iostream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include "operations/operations.hpp"
#include <readline/readline.h>
#include <readline/history.h>
#include <string>



int main(int argc, char **argv) {
//    if (argc > 1 && std::string(argv[1]) == "-d") {
//        rl_bind_key('\t', rl_insert);
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

        printf("[%s]\n", buf);
        std::istringstream ss (buf);
        std::vector<std::string> ret;

        std::copy(std::istream_iterator<std::string>(ss),
                  std::istream_iterator<std::string>(),
                  std::back_inserter(ret));

        for(auto x: ret){
            std::cout<<x<<std::endl;
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
