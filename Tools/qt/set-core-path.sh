#!/bin/sh
# Copyright (C) 2020 Konstantin Tokarev <annulen@yandex.ru>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGE.

usage()
{
    echo "Usage:"
    echo "   source $1 core_files_directory"
    echo "or"
    echo "   . $1 core_files_directory"
}

if [ "X$(basename -- "$0")" = "Xset-core-path.sh" ]; then
    echo "Error: this script should be 'sourced', not run directly"
    usage "$0"
    exit 1
fi

if [ -z "$1" ]; then
    usage set-core-path.sh
    return 1
fi

if ! [ -d "$1" ]; then
    echo "Error: core files directory '$1' does not exist"
    return 1
fi

core_dir=$(readlink -f "$1")

echo
echo "    ulimit -c unlimited"
ulimit -c unlimited

echo "    export WEBKIT_CORE_DUMPS_DIRECTORY='$core_dir'"
export WEBKIT_CORE_DUMPS_DIRECTORY="$core_dir"

kernel_core_pattern="/proc/sys/kernel/core_pattern"
new_pattern="$core_dir/core-pid_%p.dump"
old_pattern=$(cat /proc/sys/kernel/core_pattern)

if [ "$old_pattern" = "$new_pattern" ]; then
    echo
    echo "$kernel_core_pattern is already set to $new_pattern"
    return 0
fi


# Set kernel core pattern

# 1. Simple write to file if we have rights
if [ -w $kernel_core_pattern ]; then
    echo "$new_pattern" > "$kernel_core_pattern"
    if [ $? = 0 ]; then
        echo "    echo '$new_pattern' > '$kernel_core_pattern'"
        return 0
    fi
fi

# 2. Write with sudo if it's installed (may ask user for password)
if which sudo >/dev/null; then
    echo "$new_pattern" | sudo tee $kernel_core_pattern > /dev/null
    if [ $? = 0 ]; then
        echo "    echo '$new_pattern' | sudo tee $kernel_core_pattern"
        return 0
    fi
fi

# 3. Give up and print instruction

echo
echo "-----------------------------------------------"
echo "Failed to set up $kernel_core_pattern"
echo "Please run following command as root:"
echo "    echo '$new_pattern' > '$kernel_core_pattern'"
echo "-----------------------------------------------"
return 1
