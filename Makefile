make:
	gcc -o splatinfo -g splatinfo.c -lcurl -L/usr/local/lib -lcjson
install: make
	cp splatinfo /usr/local/bin/splatinfo
uninstall:
	rm /usr/local/bin/splatinfo
