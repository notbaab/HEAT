#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
# find "$DIR" -not -path "$DIR/lib/*" -name "*.cpp" -o -name "*.h" -exec clang-format -i '{}' +
# Not a bash wizard, do it in two goes
find "$DIR"  -name "*.cpp" -not -path "$DIR/lib/*" -not -path "$DIR/logger/lib/*" -not -path "$DIR/reference/*" -exec clang-format -i '{}' +
find "$DIR"  -name "*.h" -not -path "$DIR/lib/*" -not -path "$DIR/logger/lib/*" -not -path "$DIR/reference/*" -exec clang-format -i '{}' +


