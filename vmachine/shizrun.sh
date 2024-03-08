
# Директории для поиска. Не рекурсивно.
DIRS=( "data" "examples" )

# Путь по умолчанию. Заменяется, если подаются другие пути.
paths=( "./examples/fib.shasm" )
stop=true

if [ "$#" -ne 0 ]; then
    if [[ "$1" == "--help" ]]; then
        echo "  shizrun.sh [filename[.shasm]|dir/filename.shasm]..."
        exit 0
    fi
    if [ -f "$1" ]; then
        paths=( "$1" )
    else
        paths=()
        for fileToFind in $@; do
            if [[ "$fileToFind" != *.shasm ]]; then
                fileToFind+=".shasm"
            fi
            stop=false
            for dir in ${DIRS[@]}; do
                for file in $dir/*; do
                    if [ -f $file ]; then
                        if [[ "$fileToFind" == "$(basename $file)" ]]; then
                            echo SHRUN: \"${file}\" is founded.
                            paths+="$file "
                            stop=true
                        fi
                    fi
                    if $stop ; then
                        break
                    fi
                done
                if $stop ; then
                    break
                fi
            done
            if ! $stop ; then
                echo SHRUN: \"${1}\" isnot founded.
                exit 0
            fi
        done
    fi
    
fi

pathsLine=""
for path in ${paths[@]}; do
    pathsLine+="$path "
done

./bin/vmachine --exitinfo --both -o bin_shiz $pathsLine