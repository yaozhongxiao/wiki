#ifndef PYTHON_C_BRIDGE_H_
#define PYTHON_C_BRIDGE_H_

#ifdef __cplusplus
#define EXTERN_C extern "C"
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#else
#define EXTERN_C       /* Nothing */
#define EXTERN_C_BEGIN /* Nothing */
#define EXTERN_C_END   /* Nothing */
#endif

EXTERN_C void mylib_printf(char*);

#endif  // PYTHON_C_BRIDGE_H_