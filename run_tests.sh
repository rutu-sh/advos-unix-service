#!/bin/bash

function run_all_tests {
    local all_tests=$(find tests -type f -name "*.out")
    for test in $all_tests; {
        ./$test
    }
}

run_all_tests