dzftp: ftpd.o ftp_server.o ftp_client.o ftp_client_ctrlfile.o ftp_config.o request.o ftp_clientinfo.o main.o ftp_processer.o
	g++ -g -Wall -o dzftp ftp_client.o ftp_client_ctrlfile.o ftp_config.o request.o ftp_clientinfo.o main.o ftpd.o ftp_server.o ftp_processer.o
update:tags cscope.out
cscope.out: *.cpp *.h
	cscope -Rbk
tags: *.cpp *.h
	ctags -R

CPPFLAGS = -g -Wall

ftp_client.o: ftp_client.cpp global.h ftp_client.h classes.h request.h \
 ftp_client_ctrlfile.h
ftp_client_ctrlfile.o: ftp_client_ctrlfile.cpp global.h \
 ftp_client_ctrlfile.h
ftp_clientinfo.o: ftp_clientinfo.cpp global.h ftp_clientinfo.h
ftp_config.o: ftp_config.cpp global.h ftp_config.h
ftpd.o: ftpd.cpp global.h ftpd.h ftp_server.h classes.h ftp_client.h \
 ftp_config.h
main.o: main.cpp ftpd.h
request.o: request.cpp global.h request.h
ftp_processer.o: ftp_processer.cpp global.h ftp_processer.h ftp_client.h \
 classes.h ftp_clientinfo.h request.h messages.h
ftp_server.o: ftp_server.cpp global.h ftp_server.h classes.h ftp_client.h \
 ftp_config.h ftp_processer.h request.h
