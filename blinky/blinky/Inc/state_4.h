#ifndef STATE_4_H
#define STATE_4_H

/**
 * State 4: Ethernet diagnostics.
 *
 * Brings up the onboard RMII MAC/PHY path far enough to verify PHY detection
 * and current link speed without sending or receiving network traffic.
 */
void state_4_run(void);

#endif // STATE_4_H