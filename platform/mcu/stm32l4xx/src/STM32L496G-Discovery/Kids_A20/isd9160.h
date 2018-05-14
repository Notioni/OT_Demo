#ifndef __ISD9160_H
#define __ISD9160_H

int handle_upgrade(void);
void isd9160_proc_loop(void);
void isd9160_reset(void);
int isd9160_i2c_init(void);

#endif /* __ISD9160_H */
