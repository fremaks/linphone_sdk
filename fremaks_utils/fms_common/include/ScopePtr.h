#ifndef __SCOPEPTR_H__
#define __SCOPEPTR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "fms_type.h"


typedef fms_void (*clean_up)(fms_void);

class ScopePtr {
public:
	ScopePtr(clean_up *clean) {
		this->clean = clean;
	}

	~ScopePtr() {
		(*clean)();
	}
	
private:
	clean_up *clean;
};

#ifdef __cplusplus
}
#endif

#endif
