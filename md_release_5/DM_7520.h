/* 
 * File:   DM_7520.h
 * Author: longway
 *
 * Created on 1 июля 2016 г., 17:04
 */

#ifndef DM_7520_H
#define	DM_7520_H

#include "DM7520/include/dm75xx_library.h"
#include "../Common/CommonObjects.h"

#define DMA_MAX_QUEUE   8
#define MINOR_NUMBER    0


static void dma_callback(uint32_t status);

class Dma {
    dm75xx_cgt_entry_t cgt[DMA_CHANNELS_NUMBER];
    float actualRate;
    uint8_t pci_master_status;
    uint16_t *dmaDmaBuffer;
    SettingsData *settings;
//    DM75xx_Error dm75xx_status;
//    DM75xx_Board_Descriptor *rtdBoard;
public:
    uint32_t state;
    DM75xx_Error dm75xx_status;
    DM75xx_Board_Descriptor *rtdBoard;
    uint8_t inductor_state;
    int readingCount;
    uint16_t *dmaUserBuffer;
    
    ShmCommandsData command_dm75xx_err_query;
    
    Dma(SettingsData *setts);
    virtual ~Dma();

    void Start();
    void Stop();
    void Pause(bool on_off);
    int16_t* Read();
    void StartStopInductor(bool of_off);

private:

};

#endif	/* DM_7520_H */

