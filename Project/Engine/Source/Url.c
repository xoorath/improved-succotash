#include <Engine/Url.h>

#include <Engine/Log.h>
#define CURL_STATICLIB
#include <ThirdParty/Curl/curl.h>

struct eng_Url
{
	CURL* curl;
};

////////////////////////////////////////////////////////////////////////// Lifecycle

bool eng_UrlGlobalInit()
{
	CURLcode result = curl_global_init(CURL_GLOBAL_DEFAULT);
	if (result != CURLE_OK)
	{
		eng_DevFatal("curl_global_init failed with error(%d): \"%s\"", (int)result, curl_easy_strerror(result));
		return false;
	}
	return true;
}

void eng_UrlGlobalShutdown()
{
	curl_global_cleanup();
}

struct eng_Url* eng_UrlMalloc()
{
	return malloc(sizeof(struct eng_Url));
}

bool eng_UrlInit(struct eng_Url* engUrl, const char* url)
{
	engUrl->curl = curl_easy_init();
	if (engUrl->curl == NULL)
	{
		eng_DevFatal("curl_easy_init failed to return a curl handle.");
		return false;
	}

	curl_easy_setopt(engUrl->curl, CURLOPT_USERAGENT, "Mozilla/5.0...");
	curl_easy_setopt(engUrl->curl, CURLOPT_URL, url);
	CURLcode result = curl_easy_perform(engUrl->curl);

	if (result != CURLE_OK)
	{
		eng_Err("Curl perform failed for url \"%s\" with error(%d): \"%s\"", url, (int)result, curl_easy_strerror(result));
		return false;
	}

	return true;
}

void eng_UrlFree(struct eng_Url* engUrl, bool subAllocationsOnly)
{
	curl_easy_cleanup(engUrl->curl);
	if (!subAllocationsOnly)
	{
		free(engUrl);
	}
}

unsigned eng_UrlGetSizeof()
{
	return sizeof(struct eng_Url);
}
