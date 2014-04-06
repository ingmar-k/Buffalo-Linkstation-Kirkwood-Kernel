CC ?= cc

PRGS= hdddecrypt

HDDDECDIR=	/usr/local/sbin


all: $(PRGS)

hdddecrypt: hdddecrypt.c scsiaccess.c hashdata.c initio.c
	$(CC) -o $@ $?

install: $(PRGS)
	install -d $(DESTDIR)/$(HDDDECDIR)
	install $? $(DESTDIR)/$(HDDDECDIR)

clean:
	rm -f $(PRGS)
