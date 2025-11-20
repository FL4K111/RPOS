#include "os.h"
int spin_lock()
{
    c_mstatus(MSTATUS_MIE);
    return 0;
}

int spin_unlock()
{
    s_mstatus(MSTATUS_MIE);
    return 0;
}