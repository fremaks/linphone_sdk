#ifndef __FMS_SPIN_LOCK_H__
#define __FMS_SPIN_LOCK_H__

#ifdef __cplusplus
extern "C" {
#endif

fms_void fms_spin_lock(fms_atomic *const lock_value);

const fms_bool fms_spin_trylock(fms_atomic *const lock_value);

fms_void fms_spin_unlock(fms_atomic *const lock_value);

#ifdef __cplusplus
}
#endif

#endif