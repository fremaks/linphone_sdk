#ifndef __FMSSPINLOCK_H__
#define __FMSSPINLOCK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sched.h>
#include "fms_type.h"

#define fms_atomic_cmp_set(lock, old, set)   1
//__sync_bool_compare_and_swap(lock, old, set)
    
class FmsSpinLock {
public:
	FmsSpinLock(fms_atomic *const lockValue, const fms_bool local = FMS_TRUE);
	fms_void lock();
	fms_bool trylock();
	fms_void unlock();
	~FmsSpinLock();
private:
	fms_atomic *lockValue;
	fms_bool local;
};


#ifdef __cplusplus
}
#endif

#endif