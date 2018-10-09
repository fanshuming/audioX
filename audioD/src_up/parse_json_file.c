#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "cJSON.h"  //需要把该头文件放在后面包含  否则会找不到size_t

char  url[100] = "https://";

void save_version(char * url)
{
	char str[80]; 
        memcpy(str, url, strlen(url));

	//char str[] = "https://alexa.topqizhi.com/dongle/20180906.bin";
	FILE *incaptureFp = fopen("/root/version","wb");

        char *token = strtok(str, "https://alexa.topqizhi.com/dongle/");

	while(token != NULL)
	{
		printf("%s\n", token);
		fwrite(token, 1, strlen(token), incaptureFp);
		break;	//token=strtok(NULL, ",");
        }
       fclose(incaptureFp);
}

//解析JSON
char * parse_json(const char *filename)
{
 
    //从文件中读取要解析的JSON数据
    FILE *fp = fopen(filename, "r");

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *data = (char*)malloc(len + 1);
    fread(data, 1, len, fp);
    fclose(fp);

    //解析JSON数据
    cJSON *root_json = cJSON_Parse(data);    //将字符串解析成json结构体
    if (NULL == root_json)
    {
        printf("error:%s\n", cJSON_GetErrorPtr());
        cJSON_Delete(root_json);
        return NULL;
    }
    //"name":"EVDI"
    cJSON *result_json = cJSON_GetObjectItem(root_json, "result");
    if (result_json != NULL)
    {
        char *result = cJSON_Print(result_json);    //将JSON结构体打印到字符串中 需要自己释放
        printf("result:%s\n", result);
        free(result);
    }

    cJSON *message_json = cJSON_GetObjectItem(root_json, "message");
    if (message_json != NULL)
    {
        char *message = cJSON_Print(message_json);    //将JSON结构体打印到字符串中 需要自己释放
        printf("message:%s\n", message);
        free(message);
    }

    cJSON *data_json = cJSON_GetObjectItem(root_json, "data");
    if (data_json != NULL)
    {
        char *data = cJSON_Print(data_json);    //将JSON结构体打印到字符串中 需要自己释放
        if(strcmp(data, "null"))
	{
        	printf("data:%s\n", data);
        
        	strncpy(url+8, data+1, strlen(data)-2);
        	free(data);
        	printf("url:%s\n",url);
		save_version(url);

        	return url;

        }else{
		return NULL;
	}
    }
    //"data":"..."
    //id
    //username
/*
    char *username = cJSON_Print(cJSON_GetObjectItem(data_json, "username"));
    printf("username:%s\n", username);
    free(username);
    //userpass
    char *userpass = cJSON_Print(cJSON_GetObjectItem(data_json, "userpass"));
    printf("userpass:%s\n", userpass);
    free(userpass);
    //version
    char *version = cJSON_Print(cJSON_GetObjectItem(data_json, "version"));
    printf("version:%s\n", version);
*/
    //free(version);
    //free(data);
}
//创建JSON
void create_json()
{
    printf("----------------create json start-----------------------------\n");
    //组JSON
    cJSON *root_json = cJSON_CreateObject();
    cJSON_AddItemToObject(root_json, "name", cJSON_CreateString("EVDI"));
    cJSON *data_json = cJSON_CreateObject();
    cJSON_AddItemToObject(root_json, "data", data_json);
    //添加的另一种方式:cJSON_AddNumberToObject(data_json, "id", 1);通过源码发现仅仅是对cJSON_AddItemToObject的define
    cJSON_AddItemToObject(data_json, "id", cJSON_CreateNumber(1));
    //添加的另一种方式:cJSON_AddStringToObject(data_json, "username", "hahaya");
    cJSON_AddItemToObject(data_json, "username", cJSON_CreateString("hahaya"));
    cJSON_AddItemToObject(data_json, "userpass", cJSON_CreateString("123456"));
    cJSON_AddItemToObject(data_json, "version", cJSON_CreateString("1.0"));
    //打印JSON
    char *out = cJSON_Print(root_json);
    printf("%s\n", out);
    free(out);
    printf("----------------create json end-------------------------------\n");
}

/*
int main()
{
    parse_json("/tmp/page.json");
//    create_json();
    return 0;
}
*/

