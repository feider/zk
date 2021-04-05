INSTALLPATH=/usr/bin


libs=-lncurses

zk: main.c config.h cinireader.c cinireader.h
	gcc main.c cinireader.c $(libs) -o zk

install: zk
	cp zk $(INSTALLPATH)/zk

uninstall: /usr/bin/zk
	rm $(INSTALLPATH)/zk

clean: zk
	rm zk
