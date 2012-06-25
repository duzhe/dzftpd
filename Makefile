dzftpd: ftpd.o ftp_server.o ftp_ctrlfile.o ftp_config.o request.o main.o ftp_datafile.o global.o ftp_dir.o ftp_user.o log_unix.o fs_unix.o
	g++ -g -Wall -o dzftpd ftp_ctrlfile.o ftp_config.o request.o main.o ftpd.o ftp_server.o ftp_datafile.o global.o ftp_dir.o ftp_user.o log_unix.o fs_unix.o -lcrypt

clean: 
	rm *.o
cleanall:
	rm *.o dzftpd
update:tags cscope.out
cscope.out: *.cpp *.h
	cscope -Rbk
tags: *.cpp *.h
	ctags -R

CPPFLAGS = -g -Wall

fs_unix.o: fs_unix.cpp global.h classes.h log_unix.h fs_unix.h
ftp_config.o: ftp_config.cpp global.h classes.h log_unix.h fs_unix.h \
 ftp_config.h
ftp_ctrlfile.o: ftp_ctrlfile.cpp global.h classes.h log_unix.h fs_unix.h \
 ftp_ctrlfile.h
ftp_datafile.o: ftp_datafile.cpp global.h classes.h log_unix.h fs_unix.h \
 ftp_datafile.h
ftpd.o: ftpd.cpp global.h classes.h log_unix.h fs_unix.h ftpd.h \
 ftp_server.h ftp_config.h
ftp_dir.o: ftp_dir.cpp global.h classes.h log_unix.h fs_unix.h ftp_dir.h \
 ftp_user.h
ftp_server.o: ftp_server.cpp global.h classes.h log_unix.h fs_unix.h \
 ftp_server.h ftp_ctrlfile.h ftp_config.h ftp_datafile.h request.h \
 ftp_dir.h ftp_user.h messages.h
ftp_user.o: ftp_user.cpp global.h classes.h log_unix.h fs_unix.h \
 ftp_user.h
global.o: global.cpp
log_unix.o: log_unix.cpp global.h classes.h log_unix.h fs_unix.h
main.o: main.cpp global.h classes.h log_unix.h fs_unix.h ftpd.h \
 ftp_config.h
request.o: request.cpp global.h classes.h log_unix.h fs_unix.h request.h
