#!/usr/bin/bash
# Script used by GitHub workflows (mainly android)
# to clone a git repository

git clone "https://github.com/$1" --recurse-submodules --branch $2 --depth 1
