#ifndef _REQUESTS_
#define _REQUESTS_

// computes and returns a GET request string (query_params
// and cookies can be set to NULL if not needed)
char *compute_get_request(char *host, char *url, char *query_params,
							char **cookies, int cookies_count);

char *compute_get_authorisation(char *host, char *url, char *query_params,
                            char *auth);

// computes and returns a POST request string (cookies can be NULL if not needed)
char *compute_post_request(char *host, char *url, char* content_type, char **body_data,
							int body_data_fields_count, char** cookies, int cookies_count);

char *compute_post_authorisation(char *host, char *url, char* content_type, char **body_data,
                            int body_data_fields_count, char *auth);

// computes and returns a DELTE request
char *compute_delete_request(char *host, char *url, char *query_params,
                            char *auth);

#endif
