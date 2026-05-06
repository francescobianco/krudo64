#include "atlas.h"
#include "uci.h"

int main(void)
{
    atlas_init();
    uci_loop();
    return 0;
}
