dzftp: ftpd.o ftp_server.o ftp_ctrlfile.o ftp_config.o request.o main.o ftp_datafile.o global.o ftp_dir.o ftp_user.o
	g++ -g -Wall -o dzftp ftp_ctrlfile.o ftp_config.o request.o main.o ftpd.o ftp_server.o ftp_datafile.o global.o ftp_dir.o ftp_user.o 

clean: 
	rm *.o
cleanall:
	rm *.o dzftp
update:tags cscope.out
cscope.out: *.cpp *.h
	cscope -Rbk
tags: *.cpp *.h
	ctags -R

CPPFLAGS = -g -Wall

ftp_config.o: ftp_config.cpp global.h classes.h ftp_config.h
ftp_ctrlfile.o: ftp_ctrlfile.cpp global.h classes.h ftp_ctrlfile.h
ftp_datafile.o: ftp_datafile.cpp global.h classes.h ftp_datafile.h
ftp_dir.o: ftp_dir.cpp global.h classes.h ftp_dir.h
ftp_server.o: ftp_server.cpp global.h classes.h ftp_server.h \
 ftp_ctrlfile.h ftp_config.h ftp_datafile.h request.h ftp_dir.h \
 ftp_user.h messages.h
ftp_user.o: ftp_user.cpp global.h classes.h ftp_user.h
ftpd.o: ftpd.cpp global.h classes.h ftpd.h ftp_server.h ftp_config.h
global.o: global.cpp global.h classes.h
main.o: main.cpp global.h classes.h ftpd.h ftp_config.h
request.o: request.cpp global.h classes.h request.h
