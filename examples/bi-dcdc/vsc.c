/*
 * vsc.c an implementation a of Voltage Source Controller (VSC) for the DC-grid
 *
 * When load increases the current/load is shared among the available terminals
 * according according to their capacity. The higher capacity the higher load.
 * This a part of the basic droop control proposed for various grid. 
 * The parameters used in the work below is illustrates a small DC "pico" grid.
 *
 *  Robert Olsson  <robert@roolss.kth.se>  
 *                 <robert@radio-sensors.com>
 *
 *  This work is cofunded by Radio Sensors AB
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 *  Member KTH IoT grid team
 *  Project site:
 *  https://github.com/herjulf/IoT-grid.git
 */

#include <stdio.h>
#include <stdint.h>
#include "control.h"
#include "vsc.h"

/* Terminal/Converter parameter settings */

struct vsc
{
  double vdmax;	   /* Linear coeff 1: u when p=0 */
  double slope;    /* Linear coeff 2: slope of voltage droop */
  double p;        /* Current Power */
  double p_max;
  double u;        /* Terminal Voltage */
  double u_max;
  double i;        /* Terminal Current */
  double i_min;
};

struct vsc t[MTDC_MAX];

static void vsc_droop_init(struct vsc *v, int id)
{
  if(id == 0) {
    v->vdmax = VSC_MAX;
    v->slope = -0.15;
    v->p_max = 100;
  }

  if(id == 1) {
    v->vdmax = VSC_MAX;
    v->slope = -0.15;
    v->p_max = 100;
  }

  v->i_min = 0.1;
  v->u_max = v->vdmax;
  v->u = v->u_max;
}

static void vsc_droop(struct vsc *v, double i)
{

  if(i < v->i_min) {
    v->u = v->u_max;
    return;
  }

  v->u = v->vdmax / (1 - i * v->slope);
  v->p = v->u * i;

  if(v->p >= v->p_max) {
    v->p = v->p_max;
    v->u = v->p_max/i;
    return;
  }
}

void VSC_Init(void)
{
  double i;
  int p1; 

  memset(t, 0, sizeof(t));

  for(p1=0; p1 < MTDC_MAX; p1++)
    vsc_droop_init(&t[p1], p1);
}

void VSC_Calc(double i)
{
  vsc_droop(&t[0], i);
  
  set_ctrl_params(VREF, t->u);
}

#ifdef TEST
/* A simple test program */

int main()
{
  struct vsc t[MTDC_MAX];
  double i;
  int p1, p2; 

  memset(t, 0, sizeof(t));
  i = 0;

  for(p1=0; p1 < MTDC_MAX; p1++)
    vsc_droop_init(&t[p1], p1);

  for(p1=0; p1 < VSC_MAX; p1++) {

    printf("i=%-5.2f ", i);
    for(p2=0; p2 < MTDC_MAX; p2++) {
      vsc_droop(&t[p2], i);
      if(0)
	printf("p[%-d]=%-5.2f u[%-d]=%-5.2f ", p2, t[p2].p, p2, t[p2].u);
      else
	printf("%-5.2f %-5.2f ", t[p2].p, t[p2].u);
    }
    printf("\n");
    i = i + 0.7;
  }
  return 0;
}

#endif /* TEST */

int set_vsc_droop(vsc_droop_t var, float value)
{
	// return 0 if ok, return 1 if error
	switch (var)
	{

		case SLOPE:
			t[0].slope = value;
		break;

		case PMAX:
			t[0].p_max = value;
		break;
	}
	return 0;
}

float get_vsc_droop(vsc_droop_t var)
{
	float result=0;
	switch (var)
	{

		case SLOPE:
			result = t[0].slope;
		break;

		case PMAX:
			result = t[0].p_max;
		break;
	}
	return result;
}

