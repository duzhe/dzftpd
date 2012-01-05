#ifndef DZFTP_FTP_CONFIG_H_INCLUDE
#define DZFTP_FTP_CONFIG_H_INCLUDE

class ftp_config
{
public:
	unsigned short 	listen_port;
	int				max_user_counts;
	bool			allow_anonymous; 

	const char 		*default_root_path;
	const char 		*default_home_path;
	const char 		*anonymous_root_path;
	const char 		*anonymous_home_path;
public:
	ftp_config();
	int load(const char *config_file);
};

#endif
