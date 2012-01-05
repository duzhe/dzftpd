dzftp: ftpd.o ftp_server.o ftp_client.o ftp_client_ctrlfile.o ftp_config.o request.o main.o ftp_client_datafile.o global.o
	g++ -g -Wall -o dzftp ftp_client.o ftp_client_ctrlfile.o ftp_config.o request.o main.o ftpd.o ftp_server.o ftp_client_datafile.o global.o

clean: 
	rm *.o dzftp
update:tags cscope.out
cscope.out: *.cpp *.h
	cscope -Rbk
tags: *.cpp *.h
	ctags -R

CPPFLAGS = -g -Wall

ftp_client.o: ftp_client.cpp global.h classes.h ftp_client.h request.h \
 ftp_client_ctrlfile.h
ftp_client_ctrlfile.o: ftp_client_ctrlfile.cpp global.h classes.h \
 ftp_client_ctrlfile.h
ftp_client_datafile.o: ftp_client_datafile.cpp global.h classes.h \
 ftp_client_datafile.h
ftp_config.o: ftp_config.cpp global.h classes.h ftp_config.h
ftpd.o: ftpd.cpp global.h classes.h ftpd.h ftp_server.h ftp_config.h
ftp_server.o: ftp_server.cpp global.h classes.h ftp_server.h ftp_client.h \
 ftp_config.h request.h messages.h
main.o: main.cpp global.h classes.h ftpd.h ftp_config.h
request.o: request.cpp global.h classes.h request.h
global.o: global.cpp global.h
