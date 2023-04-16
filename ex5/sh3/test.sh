#!/bin/sh
cat <test.in | sort | uniq | cat >test.out

tar zcvf job5.tgz job5