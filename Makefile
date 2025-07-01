make:
	gcc -o splatinfo -g main.c -lcurl -L/usr/local/lib -lcjson
install: make
	cp -f splatinfo /usr/local/bin/splatinfo
uninstall:
	rm /usr/local/bin/splatinfo
