#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <curl/curl.h>

//url2file(binUrl, "/tmp/upgrade.bin");
//system("sysupgrade  /tmp/upgrade.bin");

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{ 


  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

//int main(int argc, char *argv[])
int url2file(char *url, char * fileName)
{
  CURL *curl_handle;
  //static const char *pagefilename = fileName;
  char *pagefilename = fileName;
  //static const char *pagefilename = "page.out";
  FILE *pagefile;

/*
  if(argc < 2) {
    printf("Usage: %s <URL>\n", argv[0]);
    return 1;
  }
*/
  printf("downing url:%s filename:%s ...\n",url,fileName);

  curl_global_init(CURL_GLOBAL_ALL);

  /* init the curl session */
  curl_handle = curl_easy_init();

  /* set URL to get here */
  curl_easy_setopt(curl_handle, CURLOPT_URL, url);

  /* Switch on full protocol/debug output while testing */
  curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);

  /* disable progress meter, set to 0L to enable and disable debug output */
  curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);

  /* send all data to this function  */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

  /* open the file */
  printf("open file:%s\n",pagefilename);

  pagefile = fopen(pagefilename, "wb");
  if(pagefile) {

    /* write the page body to this file handle */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);

    /* get it! */
    printf("writing....\n");
    curl_easy_perform(curl_handle);

    /* close the header file */
    fclose(pagefile);
  }

  /* cleanup curl stuff */
  curl_easy_cleanup(curl_handle);

  curl_global_cleanup();

  return 0;
}
