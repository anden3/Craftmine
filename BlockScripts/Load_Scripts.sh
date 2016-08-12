header=Block_Scripts.h
source=Block_Scripts.cpp

: > ${header}
: > ${source}

echo "#pragma once\n" >> ${header}

echo "#include <map>" >> ${header}
echo "#include <string>" >> ${header}
echo "#include <functional>" >> ${header}

echo "\nextern std::map<std::string, std::function<void()>> BlockFunctions;" >> ${header}

echo "\nvoid Init_Block_Scripts();" >> ${header}

echo "#include \"Block_Scripts.h\"\n" >> ${source}

for f in *.h; do
    if [ $f != $header ]; then
        echo \#include \"$f\" >> ${source}
    fi
done

echo "\nstd::map<std::string, std::function<void()>> BlockFunctions = {" >> ${source}

for f in *.h; do
    if [ $f != $header ]; then
        echo "\t{\"$(basename "$f" .h)\", $(basename "$f" .h)::Right_Click}," >> ${source}
    fi
done

echo "};" >> ${source}

echo "void Init_Block_Scripts() {" >> ${source}

for f in *.h; do
    if [ $f != $header ]; then
        echo "\t$(basename $f .h)::Init();" >> ${source}
    fi
done

echo "};" >> ${source}