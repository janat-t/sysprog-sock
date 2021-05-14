#!/usr/bin/env bash
rm -f tmp.test.txt
./ttcurl -w 5 -u /nonexistent.html -c >> tmp.test.txt &
pid1=$!

./sleep-1sec

./ttcurl -u /test.html -c >> tmp.test.txt &
pid2=$!
./ttcurl -u /test.html -c >> tmp.test.txt &
pid3=$!

./sleep-1sec

kill $pid2
kill $pid3
./sleep-1sec
kill $pid1

cat tmp.test.txt
