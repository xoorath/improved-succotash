#include <Engine/Url.h>

#include <Engine/Log.h>

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
		eng_DevFatal("curl_global_init failed with error(%d): \"%s\"\n", (int)result, curl_easy_strerror(result));
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
		eng_DevFatal("curl_easy_init failed to return a curl handle.\n");
		return false;
	}

	curl_easy_setopt(engUrl->curl, CURLOPT_USERAGENT, "Mozilla/5.0...");
	curl_easy_setopt(engUrl->curl, CURLOPT_URL, url);

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

size_t eng_UrlGetSizeof()
{
	return sizeof(struct eng_Url);
}

bool eng_UrlTestConnection(struct eng_Url* engUrl)
{
	CURLcode result = curl_easy_perform(engUrl->curl);

	switch (result)
	{
	// Success
	case CURLE_OK:
		return true;

	// Acceptable failure
	case CURLE_COULDNT_RESOLVE_HOST:
		return false;

	// Unacceptable or unknown failure
	default:
		eng_Err("Curl perform failed with error(%d): \"%s\"\n", (int)result, curl_easy_strerror(result));
		return false;
	}
}

bool eng_UrlEasyTestConnection(const char* url)
{
	struct eng_Url engUrl;
	eng_UrlInit(&engUrl, url);
	bool success = eng_UrlTestConnection(&engUrl);
	eng_UrlFree(&engUrl, true);
	return success;
}
