#include "FmsSpinLock.h"

FmsSpinLock::FmsSpinLock(fms_atomic *const lockValue, 
							    const fms_bool local) {
	this->lockValue = lockValue;
	this->local = local;

	if (local) {
		lock();
	}
}	

fms_void FmsSpinLock::lock() {
	while (1) {

		if (*lockValue == 0 && fms_atomic_cmp_set(lockValue, 0, 1)) {
	    	return;
	    }
#if 0
	    if (fms_ncpu > 1) {
	    	for (n = 1; n < spin; n <<= 1) {
				 for (i = 0; i < n; i++) {
	                ngx_cpu_pause();
	             }
	             if (*lock == 0 && ngx_atomic_cmp_set(lock, 0, 1)) {
	             	return;
	             }
	        }
	    }
#endif
	    sched_yield();//usleep(1)  may be bettr?
	}			
}

fms_bool FmsSpinLock::trylock() {
	return (*lockValue == 0 && fms_atomic_cmp_set(lockValue, 0, 1));
}
	
fms_void FmsSpinLock::unlock() {
	*lockValue = 0;		
}
	
FmsSpinLock::~FmsSpinLock() {
	if (local) {
		unlock();
	}
}
