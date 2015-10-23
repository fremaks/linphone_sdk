/*****************************************************************************
*  Copyright (C) 2015 fremaks  fremaks@163.com.                              *
*                                                                            *
*                                                                            *
*  This program is free software; you can redistribute it and/or modify      *
*  it under the terms of the GNU General Public License version 3 as         *
*  published by the Free Software Foundation.                                *
*                                                                            *
*  You should have received a copy of the GNU General Public License         *
*  along with OST. If not, see <http://www.gnu.org/licenses/>.               *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
*  @file     fms_dir.h                                                       *
*  @brief    this file include some api for handle dir                       *
*  @author   fremaks                                                         *
*  @email    fremaks@163.com                                                 *
*  @version  1.0.0                                                           *
*  @date     2015/07/11                                                      *
*  @license  GNU General Public License (GPL)                                *
*                                                                            *
*----------------------------------------------------------------------------*
*  Remark         : Description                                              *
*----------------------------------------------------------------------------*
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2015/07/11 | 1.0.0     | fremaks        | Create file                     *
*----------------------------------------------------------------------------*
*                                                                            *
*****************************************************************************/

#include "fms_dir.h"


#define DIR_ERROR(...)\
    do { \
        fprintf(stderr, "[ERR<%s,%d>]\t", __FILE__, __LINE__);\
        fprintf(stderr, __VA_ARGS__);\
    } while(0)

#define DIR_EQUAL_RETURN(arg1, arg2)\
    do {\
        if ((arg1) == (arg2)) {\
            DIR_ERROR(#arg1"=="#arg2"\n");\
			return;\
        }\
    } while(0);
	
#define DIR_EQUAL_RETURN_VALUE(arg1, arg2, ret)\
    do {\
        if ((arg1) == (arg2)) {\
            DIR_ERROR(#arg1"=="#arg2"\n");\
            return (ret);\
        }\
    }while(0);



//此模块不能使用log及其打印，因为log创建的过程本身就有用到目录创建.
const fms_bool fms_dir_is_exist(const fms_s8 *path) {
	DIR *handle = NULL;
	
	DIR_EQUAL_RETURN_VALUE(path, NULL, FMS_FALSE);

	if ((handle = opendir(path)) == NULL) {
		DIR_ERROR("opendir %s error\n", path);
		return FMS_FALSE;
	}
	closedir(handle);
	
	return FMS_TRUE;
}


//mode 好像没有完全起作用? mkdir不能递归创建目录,只能用shell命令来实现了,可惜不能得到返回值，popen也不行?
//那目前只能采取递归层层创建的办法
const fms_s32 fms_dir_create(const fms_s8 *path, mode_t mode) {

	DIR_EQUAL_RETURN_VALUE(path, NULL, FMS_FAILED);
	DIR_EQUAL_RETURN_VALUE(strlen(path) < PATH_MAX_LEN - 1, FMS_FALSE, FMS_FAILED);
	DIR_EQUAL_RETURN_VALUE(strcmp(path, "/"), 0, FMS_FAILED);
	
	if (!fms_dir_is_exist(path)) {
		fms_s8 *pos = NULL;
		fms_s8 buff[PATH_MAX_LEN] = {0};

		if ('.' == *path && '/' == *(path + 1)) {
			path += 2;
		} 
		strcpy(buff, path);

		if (buff[strlen(buff) - 1] != '/') {
			buff[strlen(buff)] = '/';
		}	
		
		pos = buff;
		while ((pos = strchr(pos, '/')) != NULL) {
			if (pos == buff) {
				pos++;
				continue;
			}
			
			*pos = '\0';
			if (!fms_dir_is_exist(buff)) {
				if (mkdir(buff, mode) != 0) { 
					DIR_ERROR("mkdir %s fail\n", buff);
					return FMS_FAILED;
				}
			}
			*pos++ = '/';
		}
	
	}
	
	return FMS_SUCCESS;
}

void fms_dir_delete(const fms_s8 *path) {

	DIR_EQUAL_RETURN(path, NULL);
	
	if (fms_dir_is_exist(path)) {
		rmdir(path);	
	}
	
}