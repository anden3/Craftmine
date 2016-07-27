header=Block_Scripts.h
source=Block_Scripts.cpp

: > ${header}
: > ${source}

echo -e "#pragma once\n" >> ${header}

echo "#include <map>" >> ${header}
echo "#include <string>" >> ${header}
echo "#include <functional>" >> ${header}

echo -e "\nextern std::map<std::string, std::function<void()>> BlockFunctions;" >> ${header}

echo -e "#include \"Block_Scripts.h\"\n" >> ${source}

for f in *.h; do
    if [ $f != $header ]; then
        echo \#include \"$f\" >> ${source}
    fi
done

echo -e "\nstd::map<std::string, std::function<void()>> BlockFunctions = {" >> ${source}

for f in *.h; do
    if [ $f != $header ]; then
        echo -e "\t{\"$(basename "$f" .h)\", $(basename "$f" .h)::Right_Click}," >> ${source}
    fi
done

echo "};" >> ${source}