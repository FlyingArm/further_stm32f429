#ifndef _CJSON_APP_H_
#define _CJSON_APP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "includes.h"
#include "malloc.h"

int create_post_login_objects(char *DeviceId,char *DeviceType,char *login_body);
int cJSON_to_str(char *json_string, char *json_string_type,char *str_val);  

#endif
       
