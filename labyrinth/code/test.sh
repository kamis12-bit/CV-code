#!/bin/bash

# Checks, whether the command was given the proper number of arguments
if [ $# -ne 2 ]; then
    echo "Wrong number of arguments";
    exit 1;
fi;


# Creates a file into which the results of the program are going to be redirected
test_filename="temp";

while [ -f "${test_filename}.out" ]
do  test_filename="${test_filename}a";
done;

test_filename="${test_filename}.out"
touch "${test_filename}";


# Counts the number of failed tests
counter=0;
for f in $2/*.in
do
# Redirects valgrind into another file, so it doesn't mess with normal output
valgrind --error-exitcode=17 \
--log-file="valgrind_logs.out" \
--leak-check=full \
--show-leak-kinds=all \
--errors-for-leak-kinds=all \
    $1 <"$f" >"${test_filename}" 2>&1;
    code=$?;
    echo -e "File ${f#$2/} ended in code ${code}. ";
    if [ ${code} -eq 0 ]; then 
        f="${f%in}out";
    else 
        f="${f%in}err";
    fi;

    # Compares the results of the program with what is expected
    if cmp --silent "${test_filename}" "$f"; then
        echo -e "Test passed. \n";
    else
        counter=$((${counter}+1));
        echo -e "Test failed: \n";
    fi;

done;

echo "Tests failed: ${counter}"

rm "${test_filename}";

exit 0;
