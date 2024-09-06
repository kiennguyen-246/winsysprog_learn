#ifndef COMMON_GUARD
#define COMMON_GUARD

#include <fltkernel.h>

#define DbgPrint(x, ...) DbgPrint("[MiniFilter]" x, __VA_ARGS__)

typedef struct _MFLT_DATA {
  // The driver object
  PDRIVER_OBJECT pDriverObject;

  // The filter obtained from FltRegisterFilter
  PFLT_FILTER pFilter;
} MFLT_DATA, *PMFLT_DATA;

#endif  // COMMON_GUARD