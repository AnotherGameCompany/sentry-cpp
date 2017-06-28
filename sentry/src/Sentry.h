#ifndef  __SENTRY_H_
#define  __SENTRY_H_


#include <iostream>
#include <string>
#include <curl/curl.h>
#include <time.h>
#include <stdlib.h>
#include <json/json.h>
#include <regex>


class Sentry
{
private:
  static std::string url;
  
  static std::string public_key;
  
  static std::string secret_key;
  
  static std::string project_id;
  
  static int timeout; 
  
  CURL *curl;
  
  Json::Value data; 
  
  struct curl_slist *headers;
  
  std::string uuid4(); 
  
  void captureMessage(std::string,
                     std::string,
                     std::string,
                     void* extra_data=NULL); 

  const char *_file;
  int _line;
  const char *_function;
  const char *_function_header;

public:
  Sentry(const char *file, int line, const char *func, const char *func_header);
  
  ~Sentry();

  void error(const char *title, const char *message=NULL, void *extra=NULL);

  void warn (const char *title, const char *message=NULL, void *extra=NULL);

  void info (const char *title, const char *message=NULL, void *extra=NULL);
  
  void debug(const char *title, const char *message=NULL, void *extra=NULL);

  static void init(std::string,int _timeout=2);
};

/// Helper macro to get file name and line number.
#define SENTRY (Sentry(__FILE__, __LINE__,__func__,__FUNCSIG__))

#endif //__SENTRY_H_
