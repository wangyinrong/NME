#define CURL_STATICLIB 1
#include <URL.h>
#include <curl/curl.h>
#include <map>
#include <Utils.h>

namespace nme
{

static CURLM *sCurlM = 0;
static int sRunning = 0;
static int sLoaders = 0;

typedef std::map<CURL *,class CURLLoader *> CurlMap;
void processMultiMessages();

CurlMap *sCurlMap = 0;

class CURLLoader : public URLLoader
{
public:
	CURL *mHandle;
	int mBytesLoaded;
	int mBytesTotal;
	URLState mState;
	int mHttpCode;
	char mErrorBuf[CURL_ERROR_SIZE];
	ByteArray *mByteArray;


	CURLLoader(const char *inURL, int inAuthType, const char *inUserPasswd,
              const char *inCookies, bool inDebug)
	{
		mState = urlInit;
		if (!sCurlM)
			sCurlM = curl_multi_init();
		mBytesTotal = -1;
		mBytesLoaded = 0;
		mHttpCode = 0;
		mByteArray = 0;
		sLoaders++;
		mHandle = curl_easy_init();
		if (!sCurlMap)
			sCurlMap = new CurlMap;
		(*sCurlMap)[mHandle] = this;
		curl_easy_setopt(mHandle, CURLOPT_URL, inURL);

      /* send all data to this function  */ 
      curl_easy_setopt(mHandle, CURLOPT_WRITEFUNCTION, staticOnData);
      curl_easy_setopt(mHandle, CURLOPT_WRITEDATA, (void *)this);
		curl_easy_setopt(mHandle, CURLOPT_NOPROGRESS, 0);
      if (inAuthType!=0)
      {
         curl_easy_setopt(mHandle, CURLOPT_HTTPAUTH, inAuthType);
         if (inUserPasswd && inUserPasswd[0])
            curl_easy_setopt(mHandle, CURLOPT_USERPWD, inUserPasswd);
      }
      curl_easy_setopt(mHandle, CURLOPT_PROGRESSFUNCTION, staticOnProgress);
      curl_easy_setopt(mHandle, CURLOPT_PROGRESSDATA, (void *)this);
		curl_easy_setopt(mHandle, CURLOPT_ERRORBUFFER, mErrorBuf );
      if (inDebug)
         curl_easy_setopt(mHandle, CURLOPT_VERBOSE, 1);
      curl_easy_setopt( mHandle, CURLOPT_COOKIEFILE, "" );
      if (inCookies && inCookies[0])
         curl_easy_setopt( mHandle, CURLOPT_COOKIE, inCookies );


      mErrorBuf[0] = '\0';
 
      /* some servers don't like requests that are made without a user-agent
         field, so we provide one */ 
      curl_easy_setopt(mHandle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

		mState = urlLoading;
		int c1 = curl_multi_add_handle(sCurlM,mHandle);
		int result = curl_multi_perform(sCurlM, &sRunning);
      processMultiMessages();

	}

	~CURLLoader()
	{
		sCurlMap->erase(mHandle);
		curl_multi_remove_handle(sCurlM,mHandle);
		curl_easy_cleanup(mHandle);
		sLoaders--;
		if (sLoaders==0)
		{
			curl_multi_cleanup(sCurlM);
			sCurlM = 0;
		}
		if (mByteArray)
			mByteArray->DecRef();
	}

	size_t onData( void *inBuffer, size_t inItemSize, size_t inItems)
	{
		size_t size = inItemSize*inItems;
		if (size>0)
		{
			if (!mByteArray)
			{
				mByteArray = new ByteArray();
				mByteArray->mBytes.resize(size);
				memcpy(&mByteArray->mBytes[0],inBuffer,size);
			}
			else
			{
				QuickVec<unsigned char> &bytes = mByteArray->mBytes;
				int s = bytes.size();
				bytes.resize(s+size);
				memcpy(&bytes[s],inBuffer,size);
			}
		}
		return inItems;
	}

	int onProgress( double inBytesTotal, double inBytesDownloaded, 
                    double inUploadTotal, double inBytesUploaded )
	{
		mBytesTotal = inBytesTotal;
		mBytesLoaded = inBytesDownloaded;
		return 0;
	}

	void setResult(CURLcode inResult)
	{
		mState = inResult==0 ? urlComplete : urlError;
	}


	static size_t staticOnData( void *inBuffer, size_t size, size_t inItems, void *userdata)
	{
		return ((CURLLoader *)userdata)->onData(inBuffer,size,inItems);
	}

	static size_t staticOnProgress(void* inCookie, double inBytesTotal, double inBytesDownloaded, 
                    double inUploadTotal, double inBytesUploaded)

	{
		return ((CURLLoader *)inCookie)->onProgress(
			inBytesTotal,inBytesDownloaded,inUploadTotal,inBytesUploaded);
	}

   void getCookies( std::vector<std::string> &outCookies )
   {
      curl_slist *list = 0;
		if (CURLE_OK == curl_easy_getinfo(mHandle,CURLINFO_COOKIELIST,&list) && list)
      {
         curl_slist *item = list;
         while(item)
         {
            outCookies.push_back(item->data);
            item = item->next;
         }
         curl_slist_free_all(list);
      }
	}
      

	URLState getState()
	{
		long http_code = 0;
		int curl_code = curl_easy_getinfo(mHandle,CURLINFO_RESPONSE_CODE,&http_code);
		if (curl_code!=CURLE_OK)
			mState = urlError;
		else if (http_code>0)
		{
			mHttpCode = http_code;
			//if (http_code>=402) mState = urlError;
			//if (http_code==200) mState = urlComplete;
		}
		return mState;
	}

	int bytesLoaded() { return mBytesLoaded; }
	int bytesTotal() { return mBytesTotal; }

	virtual int getHttpCode() { return mHttpCode; }

	virtual const char *getErrorMessage() { return mErrorBuf; }
	virtual ByteArray *releaseData()
	{
		if (mByteArray)
		{
			ByteArray *buffer = mByteArray;
			mByteArray = 0;
			return buffer;
		}
		return new ByteArray();
	}


};

void processMultiMessages()
{
		int remaining;
		CURLMsg *msg;
		while( (msg=curl_multi_info_read(sCurlM,&remaining) ) )
		{
			if (msg->msg==CURLMSG_DONE)
			{
				CurlMap::iterator i = sCurlMap->find(msg->easy_handle);
				if (i!=sCurlMap->end())
					i->second->setResult( msg->data.result );
			}
		}
}

bool URLLoader::processAll()
{
	bool check = sRunning;
	for(int go=0; go<10 && sRunning; go++)
	{
		int code = curl_multi_perform(sCurlM,&sRunning);
		if (code!= CURLM_CALL_MULTI_PERFORM)
			break;
	}
	if (check)
      processMultiMessages();

   return sRunning;
}

URLLoader *URLLoader::create(const char *inURL, int inAuthType, const char *inUserPasswd,
      const char *inCookies, bool inVerbose)
{
	return new CURLLoader(inURL,inAuthType,inUserPasswd,inCookies,inVerbose);
}



}