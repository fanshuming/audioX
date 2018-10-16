/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2015, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/
/* <DESC>
 * simple HTTP POST using the easy interface
 * </DESC>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <curl/curl.h>
#include "cJSON.h"
#include <string.h>

char * version;

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

char * get_current_version(char * versionFile)
{
    FILE *fp = fopen(versionFile, "r");

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    version = (char*)malloc(len + 1);

    fread(version, 1, len, fp);
    fclose(fp);

    printf("current version:%s\n",version);
    return version;
}

//int main(void)
int get_version()
{
  CURL *curl;
  CURLcode res;
  unsigned int len = 0;

  //data="dongle_version=20180905&mac=1C:88:79:51:EF:D0"

  static const char *pagefilename = "/tmp/version.json";
  char * current_version;
  char postData[80] = "dongle_version=";
  char postData1[80] = "";
  char postData2[80] = "";
  
  char  szMac[18];
  int	nRtn = get_mac(szMac, sizeof(szMac));
  if(nRtn > 0)
  {
        fprintf(stderr, "MAC ADDR: %s\n", szMac);
  }

  current_version = get_current_version("/root/version");

  len = strlen(current_version);
  printf("version len :%d\n", len);

  strncpy(postData+15, current_version, len-1);

  memcpy(postData1, postData, strlen(postData));
  strncpy(postData1+23,"&mac=", 5);

  printf("========postData:%s=======\n", postData1);
  memcpy(postData2, postData1, strlen(postData1));

  printf("========postData:%s=======\n", postData2);
  //strncpy(postData2+28, "1C:88:79:51:EF:D0", 17); 
  strncpy(postData2+28, szMac, strlen(szMac)); 

  printf("========postData:%s=======\n", postData2);

  FILE * pagefile;

  /* In windows, this will init the winsock stuff */
  curl_global_init(CURL_GLOBAL_ALL);

  /* get a curl handle */
  curl = curl_easy_init();
  if(curl) {
    /* First set the URL that is about to receive our POST. This URL can
       just as well be a https:// URL if that is what should receive the
       data. */
    //curl_easy_setopt(curl, CURLOPT_URL, "http://120.27.138.117:9096/dongle/device/version");
    curl_easy_setopt(curl, CURLOPT_URL, "http://dongle.topqizhi.com:9096/dongle/device/version");
    /* Now specify the POST data */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    pagefile = fopen(pagefilename, "wb");
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, pagefile);   
   

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* always cleanup */
    curl_easy_cleanup(curl);
  }

  fclose(pagefile);
  curl_global_cleanup();
  return 0;
}
