#ifndef ONR_H
#define ONR_H

#include <onn.h>

/* Default, hard-coded behaviour evaluators */

bool evaluate_light_logic(object* o, void* d);

bool evaluate_edit_rule(object* o, void* d);

bool evaluate_device_logic(object* o, void* d);

bool evaluate_clock_sync(object* o, void* d);

bool evaluate_clock(object* o, void* d);


/* The Onex Language evaluator */


#endif
