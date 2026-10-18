/*
 * Pluggable printer emulations.
 *
 * A driver is a byte pump: it receives the data stream and calls into the
 * engine to put dots on the page.  Adding a printer means writing one of
 * these and registering it in registry.c - nothing else in the core changes.
 */
#ifndef PE_DRIVER_H
#define PE_DRIVER_H

#include <stddef.h>
#include <stdint.h>

#include "pe/pe.h"

#ifdef __cplusplus
extern "C" {
#endif

/* A typeface the printer offers through ESC k, as listed on the test page. */
typedef struct {
  uint8_t id;              /* ESC k parameter                          */
  uint8_t proportional;    /* has proportional artwork                 */
  const char *name;
} pe_typeface;

typedef struct pe_driver {
  const char *id;          /* short name used on the command line/API  */
  const char *name;        /* human readable                           */
  const char *description;
  size_t state_size;       /* per job state the core allocates         */
  int32_t dot_diameter;    /* default print wire dot size, in units    */
  const pe_typeface *typefaces; /* ends with a NULL name; may be NULL  */
  void (*init)(void *state, pe_engine *e);
  void (*reset)(void *state, pe_engine *e);   /* ESC @ / hardware INIT */
  void (*byte)(void *state, pe_engine *e, uint8_t b);
  void (*eoj)(void *state, pe_engine *e);     /* end of job            */
} pe_driver;

/* Built in drivers. */
extern const pe_driver pe_driver_fx80;
extern const pe_driver pe_driver_p6;

/* Registry: NULL terminated list of every driver linked into the build. */
const pe_driver *const *pe_drivers(void);
const pe_driver *pe_driver_find(const char *id);

#ifdef __cplusplus
}
#endif
#endif /* PE_DRIVER_H */
