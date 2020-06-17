INSTALLPATH=/usr/bin

zk: main.c config.h
	gcc main.c -o zk

install: zk
	cp zk $(INSTALLPATH)/zk

uninstall: /usr/bin/zk
	rm $(INSTALLPATH)/zk

clean: zk
	rm zk
