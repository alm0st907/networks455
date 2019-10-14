#include "references/arp-send-ref.h"
// #include <linux/if_arp.h>

#define BUF_SIZ 65536
#define SEND 0
#define RECV 1

char interfaceName[IFNAMSIZ];
char buf[BUF_SIZ];
unsigned char hw_addr[6];
int debug = 0;
ssize_t numbytes;

struct arp_hdr {
  uint16_t ar_hrd;
  uint16_t ar_pro;
  unsigned char ar_hln;
  unsigned char ar_pln;
  uint16_t ar_op;
  unsigned char ar_sha[6];
  uint8_t ar_sip[4];
  unsigned char ar_tha[6];
  unsigned char ar_tip[4];
};

int main(int argc, char *argv[]) {
  int mode;
  memset(buf, 0, BUF_SIZ);

  if (argc > 3 && strcmp(argv[3], "debug") == 0) {
    debug = 1;
  }

  int correct = 0;
  if (argc > 1) {
    if (argc >= 3) {
      sscanf(argv[2], "%d.%d.%d.%d", &hw_addr[0], &hw_addr[1], &hw_addr[2],
             &hw_addr[3]);
      correct = 1;
    }

    // IFNAMESIZ defined in net/if.h so ignore lint
    strncpy(interfaceName, argv[1], IFNAMSIZ);
  }

  // console notice for how to run
  if (!correct) {
    fprintf(stderr, "./455_proj2  <InterfaceName>  <DestIP> \n");
    exit(1);
  }

  if (debug) {
    printf("Correct Args\n");
    printf("pre parse argv2: %s\n", argv[2]);
    printf("SENDER IP address: %d:%d:%d:%d\n", hw_addr[0], hw_addr[1],
           hw_addr[2], hw_addr[3]);
  }
  sendARP(argc,argv);

}
