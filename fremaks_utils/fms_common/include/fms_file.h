#ifndef __FMS_FILE_H__
#define __FMS_FILE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <unistd.h>
#include "fms_type.h"

const fms_bool fms_file_is_exist(const fms_s8 *path);

//int32_t fms_file_create(uint8_t *path, uint32_t access);

void fms_file_delete(const fms_s8 *path);


#ifdef __cplusplus
}
#endif

#endif