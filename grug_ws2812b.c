// SPDX-License-Identifier: BSD-3-Clause

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"

// The assembled pio program
#include "grug_ws2812b.pio.h"

#define NUM_PIXELS 64

// Pin assigments for LED data pin and 3 ADC pins for controls
#define OUT_PIN 2
#define H_ADC 28
#define S_ADC 27
#define V_ADC 26

// hsv_to_rgb: a minor modification of this:
// https://www.programmingalgorithms.com/algorithm/hsv-to-rgb/c/

uint32_t hsv_to_rgb(double h, double s, double v)
{
  double r = 0, g = 0, b = 0;
  unsigned char R, G, B;

  if (s == 0)
  {
    r = v;
    g = v;
    b = v;
  } else {
    int i;
    double f, p, q, t;

    if (h == 360)
      h = 0;
    else
      h = h / 60;

    i = (int)trunc(h);
    f = h - i;

    p = v * (1.0 - s);
    q = v * (1.0 - (s * f));
    t = v * (1.0 - (s * (1.0 - f)));

    switch (i) {

    case 0:
      r = v;
      g = t;
      b = p;
      break;

    case 1:
      r = q;
      g = v;
      b = p;
      break;

    case 2:
      r = p;
      g = v;
      b = t;
      break;

    case 3:
      r = p;
      g = q;
      b = v;
      break;

    case 4:
      r = t;
      g = p;
      b = v;
      break;

    default:
      r = v;
      g = p;
      b = q;
      break;
    }

  }

  R = r * 255;
  G = g * 255;
  B = b * 255;

  return (G << 24) | (R << 16) | (B << 8);
}


int main()
{
  PIO pio = pio0;
  bool ok;
  uint16_t v_adc0, v_adc1, v_adc2, i;
  uint32_t grb;
  double h = 0.0, s, v;

  // Set CPU clock speed to 128 MHz giving easy division to PIO clock of 8MHz.
  ok = set_sys_clock_khz(128000, false);
  
  stdio_init_all();

  printf("starting grug_ws2812b\n");

  if (ok) printf("clock set to %ld\n", clock_get_hz(clk_sys));

  adc_init();
  adc_gpio_init(H_ADC);
  adc_gpio_init(S_ADC);
  adc_gpio_init(V_ADC);

  uint offset = pio_add_program(pio, &grug_ws2812b_program);

  uint sm = pio_claim_unused_sm(pio, true);
  grug_ws2812b_program_init(pio, sm, offset, OUT_PIN);

  while (true) {
    adc_select_input(0);
    v_adc0 = adc_read();
    v = 1.0/4083.0*(v_adc0 - 12);

    adc_select_input(1);
    v_adc1 = adc_read();
    s = 1.0/4083.0*(v_adc1 - 12);

#undef HUE_FROM_POT
#ifdef HUE_FROM_POT
    adc_select_input(2);
    v_adc2 = adc_read();
    h = 360.0/4083.0*(v_adc2 - 12);
#else
    h = 0.0;
#endif

    for (i = 0; i < NUM_PIXELS; i++) {
      grb = hsv_to_rgb(h, s, v);
      pio_sm_put_blocking(pio, sm, grb);
      h += 360.0/NUM_PIXELS; // unused when HUE_FROM_POT
    }

    sleep_ms(100);
    
    //    printf("H: %4.1lf, S: %0.2lf, V: %0.2lf [%u %u %u]\n", h, s, v,
    //            v_adc2, v_adc1, v_adc0);
  }

  return 0;
}
