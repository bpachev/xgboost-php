#!/bin/bash

set -e
export version=$1
export module_name=xgboost
./setup $version

make clean
make
sudo make install

function find_mods_available_dir
{
    local version=$1
    local cmd=php
    if [ "$1" ]; then
        cmd=php$1
    fi
    dirname $(dirname $($cmd -r 'echo(php_ini_loaded_file());'))
}

export mods_available_dir=$(find_mods_available_dir $version)/mods-available
echo "extension=${module_name}.so" | sudo tee $mods_available_dir/$module_name.ini
if [ "$version" = "5" ]; then
    php5enmod $module_name
elif [ "$version" ]; then
    phpenmod -v $version $module_name
else
    phpenmod $module_name
fi
