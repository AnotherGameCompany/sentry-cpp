#include <Sentry/Sentry.h>
#include <RorPrerequisites.h>

std::string Sentry::url = "";
std::string Sentry::public_key = "";
std::string Sentry::secret_key = "";
std::string Sentry::project_id = "";
int Sentry::timeout = 2;

std::string& replace (std::string &str, char src)
{
    int pos = str.find_first_of (src);
    if (pos == std::string::npos)
        return str;
    return replace (str.erase (pos, 1), src);
}

Sentry::Sentry (const char* file, int line,
    const char* func, const char* func_header) :_file (file),
    _line (line),
    _function (func),
    _function_header (func_header),
    headers (NULL) {
}


void Sentry::init (std::string _url, int _timeout)
{
    //"https:\/\/(\S*):(\S*)@(\S*)\/(\d*)"
    std::regex re ("https:\\/\\/(\\S*):(\\S*)@(\\S*)\\/(\\d*)");
    std::cmatch m;
    if (std::regex_search (_url.c_str (), m, re))
    {
        public_key = m[1].str ();
        secret_key = m[2].str ();
        url = "https://" + m[3].str () + "/api/" + m[4].str () + "/store/";
        project_id = m[4].str ();
    }
    else
    {
        LOG ("regex failed!!!");
    }
    srand (time (NULL));

    timeout = _timeout;
}

Sentry::~Sentry() {
}

void Sentry::captureMessage (std::string title,
    std::string message,
    std::string level,
    void* extra_data)
{
    char auth_head[2018] = { 0, };
    CURLcode res;
    snprintf (auth_head, 1024, "X-Sentry-Auth: Sentry sentry_version=5, sentry_timestamp=%llu, \
                           sentry_key=%s, sentry_client=raven-cpp/0.1,\
                           sentry_secret=%s", time (NULL), public_key.c_str (), secret_key.c_str ());
    headers = curl_slist_append (headers, auth_head);
    headers = curl_slist_append (headers, "User-Agent: raven-cpp/0.1");
    headers = curl_slist_append (headers, "Content-Type: application/octet-stream");
    time_t rawtime;
    struct tm *timeinfo;
    char timebuf[100] = { 0, };
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    strftime (timebuf, 100, "%Y-%m-%d %H:%M:%S", timeinfo);
    data["project_id"] = project_id;
    data["event_id"] = uuid4 ();
    data["culprit"] = title;
    data["timestamp"] = timebuf;
    data["message"] = message;
    data["level"] = level;
    data["platform"] = "c/c++";
    Json::Value extra_val;
    if (extra_data != NULL)
    {
        extra_val = *(static_cast<Json::Value *>(extra_data));
    }
    extra_val["current file"] = _file;
    extra_val["current line"] = _line;
    extra_val["current function"] = _function;
    extra_val["current function declare"] = _function_header;
    data["extra"] = extra_val;
    char sbuf[50] = { 0, };
    int retval = gethostname (sbuf, 50);
    if (0 == retval)
    {
        data["server_name"] = sbuf;
    }
    CURLM *multi_handle = curl_multi_init ();
    curl = curl_easy_init ();
    if (curl)
    {
        Json::FastWriter writer;
        std::string data_str = writer.write (data);
        LOG (url.c_str ());
        curl_easy_setopt (curl, CURLOPT_POST, 1);
        curl_easy_setopt (curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt (curl, CURLOPT_URL, url.c_str ());
        curl_easy_setopt (curl, CURLOPT_POSTFIELDS, data_str.c_str ());
        curl_easy_setopt (curl, CURLOPT_VERBOSE, false);
        curl_easy_setopt (curl, CURLOPT_CONNECTTIMEOUT, timeout);
        curl_easy_setopt (curl, CURLOPT_TIMEOUT, timeout);
        res = curl_easy_perform(curl);
        long http_code = 200;
        curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
        LOG ("[Sentry] curl returned: " + std::to_string(http_code));
    }
    else
    {
        LOG ("[Sentry] curl is null");
    }
    curl_multi_cleanup (multi_handle);
    curl_easy_cleanup (curl);
}

/*
 * uuid
 */
std::string Sentry::uuid4 () {
    char uuid[1024] = { 0, };
    snprintf (uuid, 1024, "%04x%04x-%04x-%04x-%04x-%04x%04x%04x", rand () % 0xffff, \
        rand () % 0xffff, \
        rand () % 0xffff, \
        rand () % 0x0fff | 0x4000, \
        rand () % 0x3fff | 0x8000, \
        rand () % 0xffff, \
        rand () % 0xffff, \
        rand () % 0xffff);
    std::string tmp = uuid;
    replace (tmp, '-');
    return tmp;
}

void Sentry::error (const char *title, const char *message, void *extra) {
    if (message == NULL)
    {
        captureMessage (title, title, "error", extra);
    }
    else
    {
        captureMessage (title, message, "error", extra);
    }
}

void Sentry::warn (const char *title, const char *message, void *extra) {
    if (message == NULL)
    {
        captureMessage (title, title, "warn", extra);
    }
    else
    {
        captureMessage (title, message, "warn", extra);
    }
}

void Sentry::debug (const char* title, const char *message, void *extra) {
    if (message == NULL)
    {
        captureMessage (title, title, "debug", extra);
    }
    else
    {
        captureMessage (title, message, "debug", extra);
    }
}

void Sentry::info (const char* title, const char *message, void *extra) {
    if (message == NULL)
    {
        captureMessage (title, title, "info", extra);
    }
    else
    {
        captureMessage (title, message, "info", extra);
    }
}
