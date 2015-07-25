/**
 * @file
 * @brief
 *
 * @author  Anton Kozlov
 * @date    15.04.2015
 */

#ifndef HTTPD_H_
#define HTTPD_H_

#include <stdio.h>
#include <sys/socket.h>

#include "httpd_log.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) \
	(sizeof(array) / sizeof(*(array)))
#endif

#define HTTPD_MAX_PATH 128
#define CGI_PREFIX  "/cgi-bin/"

struct client_info {
	struct sockaddr ci_addr;
	socklen_t ci_addrlen;
	int ci_sock;
	int ci_index;

	const char *ci_basedir;
};

struct http_req_uri {
	char *target;
	char *query;
};

struct http_req {
	struct http_req_uri uri;
	char *method;
	char *content_len;
	char *content_type;
};

extern char *httpd_parse_request(char *str, struct http_req *hreq);
extern int httpd_build_request(struct client_info *cinfo, struct http_req *req, 
		char *buf, size_t buf_sz);

extern pid_t httpd_try_respond_script(const struct client_info *cinfo, const struct http_req *hreq);
extern pid_t httpd_try_respond_cmd(const struct client_info *cinfo, const struct http_req *hreq);
extern int httpd_try_respond_file(const struct client_info *cinfo, const struct http_req *hreq,
		char *buf, size_t buf_sz);

extern const char *httpd_filename2content_type(const char *filename);
extern int httpd_header(const struct client_info *cinfo, int st, const char *msg);

#endif /* HTTPD_H_ */


