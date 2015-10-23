#include "fms_file.h"
#include "fms_log.h"


const fms_bool fms_file_is_exist(const fms_s8 *path) {

	FMS_EQUAL_RETURN_VALUE(path, NULL, FMS_FALSE);

	if (access(path, F_OK) != 0) {
		FMS_ERROR("access %s error\n", path);
		return FMS_FALSE;
	}

	return FMS_TRUE;
}

void fms_file_delete(const fms_s8 *path) {

	FMS_EQUAL_RETURN(path, NULL);

	if (fms_file_is_exist(path)) {
		remove(path);
	}
	
}