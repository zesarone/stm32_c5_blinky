#ifndef ETHERNET_DIAG_H
#define ETHERNET_DIAG_H

/**
 * State 4 Ethernet diagnostics.
 *
 * Performs the low-level MAC/MDIO bring-up needed to probe the onboard
 * LAN8742A PHY and displays the resulting link status on the 7-segment module.
 */
void ethernet_diag_run(void);

#endif // ETHERNET_DIAG_H