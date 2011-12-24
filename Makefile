dzftp: ftp_client.o ftp_client_ctrlfile.o ftp_config.o ftpd.o ftp_server.o main.o request.o
	g++ -o ftp_client.o ftp_client_ctrlfile.o ftp_config.o ftpd.o ftp_server.o main.o request.o
update:tags cscope.out
cscope.out: *.cpp *.h
	cscope -Rbk
tags: *.cpp *.h
	ctags -R


ftp_client.o: ftp_client.cpp global.h ftp_client.h request.h
ftp_client_ctrlfile.o: ftp_client_ctrlfile.cpp global.h ftp_client_ctrlfile.h
ftp_config.o: ftp_config.cpp global.h ftp_config.h
ftpd.o: ftpd.cpp global.h ftpd.h ftp_server.h ftp_client.h
ftp_server.o: ftp_server.cpp global.h ftp_server.h ftp_client.h ftp_config.h request.h
main.o: main.cpp ftpd.h
request.o: request.cpp global.h request.h
