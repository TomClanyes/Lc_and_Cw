#!/bin/bash
long_args_str="help,users,processes,log:,errors:"
short_args_str="hupl:e:"

help_flag=0
users=0
processes=0
out_path=""
err_path=""

cat="/usr/bin/cat"
sort="/usr/bin/sort"
sed="/usr/bin/sed"
cut="/usr/bin/cut"
ps="/usr/bin/ps"
touch="/usr/bin/touch"

# Shows help menu
show_help() {
    echo -e "$0 [PARAMETERS]"
    echo -e "--help\t\t-h\t\tshows this help message"
    echo -e "--users\t\t-u\t\tshows users list and their home directories"
    echo -e "--processes\t-p\t\tlist process sorted by PID"
    echo -e "--log [PATH]\t-l [PATH]\tredirects stdout to specified file"
    echo -e "--errors [PATH]\t-e [PATH]\tredirects stderr to specified file"
}

# Parses arguments takes arguments $@ as positional parameter or as its own $@
parse_arguments() {
    # Check if arguments were passed
    if [ $# -eq 0 ]; then
        echo "No arguments were passed" >&2
        show_help
        exit 1
    fi

    # Parsing options using getopt and globaly defined variables with arguments names
    local options=$(getopt -o $short_args_str -l $long_args_str -- "$@")
    # If error occured while parsing arguments (getopt returned not 0)
    if [ $? -ne 0 ]; then
        echo "Error while parsing arguments" >&2
        show_help
        exit 1
    fi
    eval set -- "$options"

    # Read the named argument values
    while [ $# -gt 0 ]; do
        case "$1" in
            "-h" | "--help")
                help_flag=1;;
            "-u" | "--users")
                users=1;;
            "-p" | "--processes")
                processes=1;;
            "-l" | "--log")
                out_path="$2"
                shift;;
            "-e" | "--errors")
                err_path="$2"
                shift;;
            "-" | "--")
                shift;;
            "*")
                echo "Unknown argument $1" >&2
                exit 1;;
        esac
        shift
    done
}

# Formats /etc/passwd and lists all users and thir home directories
get_users() {
    # Reading, sorting, cutting first and last fields (username and directory)
    local users=$($cat /etc/passwd | $sort | $cut -d : -f 1,6)

    printf "%-32s%s\n" "USER" "HOME" >&1
    for line in $users
    do
        printf "%-32s%s\n" $($sed "s/:/\n/g" <<< $line) >&1
    done
}

# Lists all available by user processes
get_processes() {
    $ps -a
}

# Takes positional argument $1 as file path
create_file() {
    # Trying create new file
    $touch $1
    # If touch fails that means file is inaccessable 
    if [ $? -ne 0 ]; then
        echo "$1 is not writable" >&2
        exit 1
    fi
}

###################################
#             Entry               #
###################################
parse_arguments $@

# Trying to redirect whole output to specified path
if [ -n "$out_path" ]; then
    echo "Redirecting stdout to $out_path"
    create_file $out_path
    exec 1>$out_path
fi

# Trying to redirect errors output to specified path
if [ -n "$err_path" ]; then
    echo "Redirecting stderr to $err_path"
    create_file $err_path
    exec 2>$err_path
fi

# Validating options
if [ $(($help_flag + $users + $processes)) -ne 1 ]; then
    echo "Only one option must be used" >&2
    exit 1
fi

# Executing specified options
if [ $help_flag -eq 1 ]; then
    show_help
elif [ $users -eq 1 ]; then
    get_users
elif [ $processes -eq 1 ]; then
    get_processes
fi
exit 0
