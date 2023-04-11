#!/bin/sh
cat <test.in | sort | uniq | cat >test.out
cat <test.in | cat >test.out
cat <test.in | uniq