#!/bin/bash

ProgName=$(basename $0)
CPP_EXTENSIONS="cpp|cc|cxx|hpp|h"
  
sub_help(){
    echo "Usage: $ProgName <subcommand> [options]\n"
    echo "Subcommands:"
    echo "    build        Build the library"
    echo "    clean        Clean all build artifacts"
    echo "    cpplint      Lint all of the c++ code using cpplint"
    echo "    format       Format all code using flake8 and clang-format"
    echo "    format-cpp   Format c/c++ code using clang-format"
    echo "    format-py    Format python code using flake8"
    echo ""
    echo "For help with each subcommand run:"
    echo "$ProgName <subcommand> -h|--help"
    echo ""
}

sub_cpplint() { 
    original_wd=$(pwd)

    cd "$(git rev-parse --show-toplevel)"

    fd ".*\.($CPP_EXTENSIONS)" ./src/ ./include/ ./tests/ | xargs cpplint

    cd "$original_wd"
}

sub_build() { 
    original_wd=$(pwd)

    cd "$(git rev-parse --show-toplevel)"

    if [ -d ./build/ ]; then
        cd ./build/
    else
        mkdir ./build/
        cd ./build/
    fi

    cmake ..

    make -j8

    cd "$original_wd"
}

sub_clean() { 
    original_wd=$(pwd)

    cd "$(git rev-parse --show-toplevel)"

    if [ -d ./build/ ]; then
        rm -rf ./build/
    fi

    rm ./cppmpc_py/_cppmpc_swig.so

    cd "$original_wd"
}

sub_format() {
    sub_format-py
    sub_format-cpp
}
  
sub_format-cpp(){
    original_wd=$(pwd)

    cd "$(git rev-parse --show-toplevel)"
    fd ".*\.($CPP_EXTENSIONS)" ./include/ ./src/ ./tests/ | \
        xargs clang-format -i

    cd "$original_wd"
}

sub_format-py(){
    original_wd=$(pwd)

    cd "$(git rev-parse --show-toplevel)"
    fd ".*\.(py)" ./tests/ | \
        xargs black

    cd "$original_wd"
}
 
subcommand=$1
case $subcommand in
    "" | "-h" | "--help")
        sub_help
        ;;
    *)
        shift
        sub_${subcommand} $@
        if [ $? = 127 ]; then
            echo "Error: '$subcommand' is not a known subcommand." >&2
            echo "       Run '$ProgName --help' for a list of known subcommands." >&2
            exit 1
        fi
        ;;
esac
