SRCS=main.c logger.c ttcurl.c sleep-1sec.c bin-cgi.c
HDRS=main.h
EXES=main ttcurl sleep-1sec docroot/bin.cgi
SCRIPTS=test.sh
TXTS=report.txt
DOCS=docroot/index.html docroot/test.html docroot/script.cgi
TESTS=t/2-wait-out.txt t/2-wait.sh t/200.txt t/404.txt t/500.txt t/bin-cgi.txt t/script-cgi.txt
INST_URL=https://se.c.titech.ac.jp/lecture/sysprog/cmd

all: $(EXES)
.PHONY: all download clean zip test test1 test2 testA

URL=https://se.c.titech.ac.jp/lecture/sysprog/4-sock
DLS=$(SRCS) $(HDRS) $(TXTS) $(SCRIPTS) $(DOCS) $(TESTS)
download:
	-mkdir -p docroot t
	for x in $(DLS); do [ -f $$x ] || curl -u sysprog:int128 $(URL)/$$x -o $$x; done
	chmod ugo+x docroot/script.cgi

REPORTS=$(SRCS) $(HDRS) report.txt
zip: report-sock.zip
report-sock.zip: $(REPORTS)
	zip $@ $+

CC=clang
CFLAGS=-std=gnu99 -Wall -Werror
LDFLAGS=
main: main.c logger.c $(HDRS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ main.c logger.c
ttcurl: ttcurl.c logger.c $(HDRS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ ttcurl.c logger.c
sleep-1sec: sleep-1sec.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ sleep-1sec.c
docroot/bin.cgi: bin-cgi.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ bin-cgi.c

clean:
	-$(RM) -f $(EXES)

OPT=
test: test1 test2 testA
test1: $(EXES)
	@sh test.sh $(OPT) -k 1-1test-code  -o t/200.txt          ./ttcurl -u /test.html -c
	@sh test.sh $(OPT) -k 1-2test-body  -o docroot/test.html  ./ttcurl -u /test.html -b
	@sh test.sh $(OPT) -k 1-3index-code -o t/200.txt          ./ttcurl -u / -c
	@sh test.sh $(OPT) -k 1-4index-body -o docroot/index.html ./ttcurl -u / -b
	@sh test.sh $(OPT) -k 1-5notfound   -o t/404.txt          ./ttcurl -u /nonexistent.html -c

test2: $(EXES)
	@sh test.sh $(OPT) -k 2-wait -o t/2-wait-out.txt sh t/2-wait.sh

testA: $(EXES)
	@sh test.sh $(OPT) -k A-1bin         -o t/bin-cgi.txt    ./ttcurl -u /bin.cgi -b
	@sh test.sh $(OPT) -k A-2script      -o t/script-cgi.txt ./ttcurl -u /script.cgi -b
	@sh test.sh $(OPT) -k A-3bin-code    -o t/200.txt        ./ttcurl -u /bin.cgi -c
	@sh test.sh $(OPT) -k A-4script-code -o t/200.txt        ./ttcurl -u /script.cgi -c
	@sh test.sh $(OPT) -k A-5none-code   -o t/404.txt        ./ttcurl -u /nonexistent.cgi -c 

install-commands:
	type telnet  >/dev/null || curl -u sysprog:int128 $(INST_URL)/install.telnet.sh | sh
	type tmux    >/dev/null || curl -u sysprog:int128 $(INST_URL)/install.tmux.sh   | sh
