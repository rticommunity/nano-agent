/******************************************************************************
 *
 * (c) 2020 Copyright, Real-Time Innovations, Inc. (RTI) All rights reserved.
 *
 * RTI grants Licensee a license to use, modify, compile, and create derivative
 * works of the Software solely in combination with RTI Connext DDS. Licensee
 * may redistribute copies of the Software provided that all such copies are
 * subject to this License. The Software is provided "as is", with no warranty
 * of any type, including any warranty for fitness for any purpose. RTI is
 * under no obligation to maintain or support the Software. RTI shall not be
 * liable for any incidental or consequential damages arising out of the use or
 * inability to use the Software. For purposes of clarity, nothing in this
 * License prevents Licensee from using alternate versions of DDS, provided
 * that Licensee may not combine or link such alternate versions of DDS with
 * the Software.
 *
 ******************************************************************************/


#include "dds_agent/dds_agent.h"

RTILogBitmap D2S2Log_g_submoduleMask = D2S2_SUBMODULE_MASK_ALL;


RTILogBitmap D2S2Log_g_instrumentationMask = RTI_LOG_BITMAP_DEFAULT;

void
D2S2Log_setBitmaps(
    RTILogBitmap submoduleMask,
    RTILogBitmap instrumentationMask)
{
    D2S2Log_g_submoduleMask       = submoduleMask;
    D2S2Log_g_instrumentationMask = instrumentationMask;
}

void D2S2Log_getBitmaps(RTILogBitmap *submoduleMask,
                            RTILogBitmap *instrumentationMask)
{
    *submoduleMask       = D2S2Log_g_submoduleMask;
    *instrumentationMask = D2S2Log_g_instrumentationMask;
}

void D2S2Log_setVerbosity(const RTILogBitmap verbosity)
{
    D2S2Log_g_instrumentationMask = verbosity;
}