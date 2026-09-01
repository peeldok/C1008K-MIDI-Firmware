/* Boot animation player (supplied frame stream). */
#ifndef C100_BOOT_ANIM_H
#define C100_BOOT_ANIM_H

/* Plays the animation to completion, then clears. Blocking.
 * `tick` is called every ~1 ms during waits (keep USB alive: pass tud_task). */
void boot_anim_play(void (*tick)(void));

#endif
