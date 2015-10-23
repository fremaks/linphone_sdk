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

#ifndef __FMS_DIR_H__
#define __FMS_DIR_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include "fms_type.h"
#define PATH_MAX_LEN          256

/** 
 * @brief Determine whether a specified directory exists
 * @param [in] path    the path of specified directory
 * @return  
 *		-<em>FMS_FALSE</em> fail
 *      -<em>FMS_TRUE</em>  succeed      
 */
const fms_bool fms_dir_is_exist(const fms_s8 *path);

/** 
 * @brief create a specified directory 
 * @param [in] path    the path of specified directory
 * @return  
 *		-<em>FMS_FAILED</em>   fail
 *       -<em>FMS_SUCCESS</em>  succeed      
 */
const fms_s32 fms_dir_create(const fms_s8 *path, mode_t mode);

/**
 * @brief Take a permanent object and make it eligible for freedom.
 * @param path [in]   The bigint to be made back to temporary.
 */
fms_void fms_dir_delete(const fms_s8 *path);


#ifdef __cplusplus
}
#endif

#endif