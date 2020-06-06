zk: main.c config.h
	gcc main.c -o zk

install: zk
	cp zk /usr/bin/zk

uninstall: /usr/bin/zk
	rm /usrl/bin/zk

clean: zk
	rm zk
