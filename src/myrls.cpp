#include <iostream>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pwd.h>
#include <vector>
#include <tuple>
#include <algorithm>

std::string to_lowercase(std::string &word){
    for(auto &c: word){
        c = std::tolower(c);
    }
    return word;
}

bool string_comparison(std::string lhs, std::string rhs){
    std::string lhs_low = to_lowercase(lhs);
    std::string rhs_low = to_lowercase(rhs);
    if (lhs_low == rhs_low)
        return rhs < lhs;
    return lhs_low < rhs_low;
}

struct File {
    std::string fpath;
    struct stat sb;
    int tflag;
    struct FTW ftwbuf;

    bool operator<(File const& rhs) const {
        if (fpath.substr(0, ftwbuf.base) == rhs.fpath.substr(0, rhs.ftwbuf.base)){
            return string_comparison((std::string)(fpath.c_str() + ftwbuf.base), (std::string)(rhs.fpath.c_str() + rhs.ftwbuf.base));
        }
        else if (ftwbuf.level == rhs.ftwbuf.level){
            return string_comparison(fpath.substr(0, ftwbuf.base), rhs.fpath.substr(0, rhs.ftwbuf.base));
        }
        else {
            return ftwbuf.level < rhs.ftwbuf.level;
        }
    }
};

char* permissions(const struct stat *sb){
    char *access_modes = static_cast<char *>(malloc(10));
    access_modes[0] = (sb->st_mode & S_IRUSR) ? 'r' : '-';
    access_modes[1] = (sb->st_mode & S_IWUSR) ? 'w' : '-';
    access_modes[2] = (sb->st_mode & S_IXUSR) ? 'x' : '-';
    access_modes[3] = (sb->st_mode & S_IRGRP) ? 'r' : '-';
    access_modes[4] = (sb->st_mode & S_IWGRP) ? 'w' : '-';
    access_modes[5] = (sb->st_mode & S_IXGRP) ? 'x' : '-';
    access_modes[6] = (sb->st_mode & S_IROTH) ? 'r' : '-';
    access_modes[7] = (sb->st_mode & S_IWOTH) ? 'w' : '-';
    access_modes[8] = (sb->st_mode & S_IXOTH) ? 'x' : '-';
    access_modes[9] = '\0';

    return access_modes;
}

char check_filetype(const struct stat *sb){
    char filetype = '\0';
    if (S_ISDIR(sb->st_mode)) {
        filetype = '/';
    } else if (S_ISLNK(sb->st_mode)) {
        filetype = '@';
    } else if (S_ISFIFO(sb->st_mode)) {
        filetype = '|';
    } else if (S_ISSOCK(sb->st_mode)) {
        filetype = '=';
    } else if (!S_ISREG(sb->st_mode)) {
        filetype = '?';
    } else if (S_IXUSR & sb->st_mode) {
        filetype = '*';
    }

    return filetype;
}

static int display_info(std::vector<File> &fv){
    std::sort(fv.begin(), fv.end());

    std::string prev_dir;
    std::string cur_dir;

    for (auto f : fv) {
        char buf[200];
        struct tm *tm;
        tm = localtime(&(f.sb.st_mtime));
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm);

        cur_dir = f.fpath.substr(0, f.ftwbuf.base);

        if (prev_dir != cur_dir) {
            std::cout << "\n" << cur_dir << ":" << std::endl;
        }

        printf("%s %s %7jd %s %c%s\n",
               permissions(&f.sb),
               getpwuid(f.sb.st_uid)->pw_name, (intmax_t) f.sb.st_size,
               buf, check_filetype(&f.sb), f.fpath.c_str() + f.ftwbuf.base);

        prev_dir = cur_dir;
    }
    return 0;
}

std::vector<File> files;

int files_to_vector(const char *fpath, const struct stat *sb,
                    int tflag, struct FTW *ftwbuf)
{
    if (!strcmp(fpath, ".")) {
        return 0;
    }

    files.push_back((File){fpath, *sb, tflag, *ftwbuf});
    return 0;
}

int main(int argc, char **argv) {
    int flags = FTW_MOUNT | FTW_PHYS | FTW_DEPTH;
    
    if (nftw((argc < 2) ? "." : argv[1], files_to_vector, 20, flags) == -1)
    {
        perror("nftw");
        exit(EXIT_FAILURE);
    }
    display_info(files);

    exit(EXIT_SUCCESS);
}