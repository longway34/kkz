/* 
 * File:   DM_7520.cpp
 * Author: longway
 * 
 * Created on 1 июля 2016 г., 17:04
 */

#include <condition_variable>
#include <error.h>

#include "DM_7520.h"
#include "Loggers.h"
//#include "Loggers.cpp"

uint16_t *pDmaBuffer = NULL;
std::condition_variable readyToRead;
DM75xx_Error dm75xx_error;
static Dma *dmaSelf = NULL;
uint dmaIrqCount = 0;                           // 1 - DMA_MAX_QUEUE (включительно)
uint16_t *dma_u_buffers[DMA_MAX_QUEUE];           // таблица возможных указателей
static uint16_t *dmaData;                        // пользовательский буфер
std::mutex queueMutex;

extern bool noConsole;
extern ILogger *logger;

#ifdef DEBUG
    int toPort1 = 0;
#endif

extern int dm75_chn[];
    
void dma_callback(uint32_t status){
    if(status != DM75xx_INT_DMA_0){
        fprintf(stdout, "status=%x\n",status);
    }
    if (DM75xx_INTERRUPT_ACTIVE(status, DM75xx_INT_DMA_0)) {
#ifdef DEBUG
        toPort1 |= 0x10;
        DM75xx_DIO_Set_Port(dmaSelf->rtdBoard, DM75xx_DIO_PORT1, toPort1);
#endif
        dmaIrqCount++;
        if(dmaIrqCount > DMA_MAX_QUEUE){
            dmaIrqCount = 1;
        }
        pDmaBuffer = DM75xx_DMA_Buffer_Read_Me(dmaSelf->rtdBoard, DM75xx_DMA_CHANNEL_0,dmaIrqCount,&dm75xx_error);

        DM75xx_Return_Status(dmaSelf->rtdBoard, dm75xx_error, "DM75xx_DMA_Buffer_Read_Me");
#ifdef DEBUG            
            if(pDmaBuffer != dma_u_buffers[dmaIrqCount-1]){
                fprintf(stdout, "CALL BACK FUNC::Error return reading DMA buffer %x(%x)", pDmaBuffer, dma_u_buffers[dmaIrqCount-1]);
                dma_u_buffers[dmaIrqCount - 1] = pDmaBuffer;
            }
        toPort1 &= ~(0x10);
        DM75xx_DIO_Set_Port(dmaSelf->rtdBoard, DM75xx_DIO_PORT1, toPort1);
#endif
        
        readyToRead.notify_all();


    } else if (DM75xx_INTERRUPT_ACTIVE(status, DM75xx_INT_DIO)) {
        uint8_t dio_value;
        DM75xx_DIO_Get_Port(dmaSelf->rtdBoard,DM75xx_DIO_PORT0, &dmaSelf->inductor_state);
        DM75xx_Return_Status(dmaSelf->rtdBoard, dm75xx_error,
                             "DM75xx_DIO_Get_Port");
        /* 
         * 
         * Clear Digital I/O IRQ
         */
        dm75xx_error = DM75xx_DIO_Clear_IRQ(dmaSelf->rtdBoard);
        DM75xx_Return_Status(dmaSelf->rtdBoard, dm75xx_error,
                             "DM75xx_DIO_Clear_IRQ");
        
    }

}

int16_t* Dma::Read(){
    int16_t *res = NULL;
//            fprintf(stdout, "-");

#ifdef DEBUG    
        if((toPort1 & 0x20) == 0) {
            toPort1 = toPort1 | 0x20; 
        }
        else {
            toPort1 = toPort1 & ~(0x20);
        }
        DM75xx_DIO_Set_Port(rtdBoard, DM75xx_DIO_PORT1, toPort1);
#endif
        
    std::unique_lock<std::mutex> lck(queueMutex);
    while(rtdBoard && readingCount == dmaIrqCount){
            readyToRead.wait(lck);
    }
    if(rtdBoard){
        readingCount++; 
        if(readingCount > DMA_MAX_QUEUE) readingCount = 1;


        if(readingCount != dmaIrqCount){
#ifdef DEBUG
            int m = dmaIrqCount - readingCount;
            if(m < 0) m = DMA_MAX_QUEUE - m;
            command_dm75xx_err_query.parameter = m;
            if(!noConsole){
                fprintf(stdout, "*********** Posible ERROR reading DMA Buffers rerdCounts %d dmaCounts %d ***********************\n", readingCount, dmaIrqCount);
            }
            if(logger){
                logger->addLoggerState(command_dm75xx_err_query);
            }
#endif                
            state = ERR_READER_QUERY;
        } else {
            state = STATE_OK;
        }
//        if(dmaIrqCount == 1)
//            fprintf(stdout, "*");
        res = (int16_t*)dma_u_buffers[readingCount - 1];
#ifdef DEBUG
    if((toPort1 & 0x20) == 0) {
        toPort1 = toPort1 | 0x20; 
    }
    else {
        toPort1 = toPort1 & ~(0x20);
    }
    DM75xx_DIO_Set_Port(rtdBoard, DM75xx_DIO_PORT1, toPort1);
#endif            
        return res;
    }
#ifdef DEBUG           
    fprintf(stdout, "Error ADC not Installing...\n");
#endif
}

Dma::Dma(SettingsData *_setts) {
    settings = _setts;
    dmaSelf = this;
    rtdBoard = NULL;
    command_dm75xx_err_query.commandId = ERR_DMA_READ_QUERY;
    command_dm75xx_err_query.parameter = 0;
}

Dma::~Dma() {
    Stop();
#ifdef DEBUG
    if(!noConsole)
	fprintf(stdout, "Closing the dm75xx... \n");
#endif
	/*
	 * Reset the board.
	 */
#ifdef DEBUG
    if(!noConsole)
	fprintf(stdout, "Board Reset... \n");
#endif
    dm75xx_status = DM75xx_Board_Reset(rtdBoard);
    DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_Board_Reset ");
	/*
	 * Close the dm75xx device.
	 */
    dm75xx_status = DM75xx_Board_Close(rtdBoard);
    DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_Board_Close ");
        
    rtdBoard = NULL;
}

void Dma::Pause(bool on_off){
//    StartStopInductor(on_off);
    if(on_off){
        DM75xx_PCLK_Stop(rtdBoard);
#ifdef DEBUG
    if(!noConsole)
        fprintf(stdout, "Stoping Pacer Clock... \n");
#endif
    } else {
#ifdef DEBUG
    if(!noConsole)
        fprintf(stdout, "Staring Pacer Clock... \n");
#endif
        DM75xx_PCLK_Start(rtdBoard);
    }
}

void Dma::Start(){
/*
 * Opening board
 */
readingCount = 0;
dmaIrqCount = 0;

#ifdef DEBUG   
    if(!noConsole)
        fprintf(stdout, "Opening dm75xx with minor number %d ...\n",
		MINOR_NUMBER);
#endif
if(!rtdBoard){
    dm75xx_status = DM75xx_Board_Open(MINOR_NUMBER, &rtdBoard);
    DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_Board_Open");
	/*
	 * Initialize DMA on channel 0
	 */
}
    if(rtdBoard){
            /*
             * Initialize the board.
             */
	/*
	 * Initialize the board.
	 */
#ifdef DEBUG
        if(!noConsole)
            fprintf(stdout, "Board Initialization ...\n");
#endif
        dm75xx_status = DM75xx_Board_Init(rtdBoard);
        DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_Board_Init");
	/*
	 * Check the boards PCI Master status.
	 */
#ifdef DEBUG
        if(!noConsole)
            fprintf(stdout, "Checking PCI Master status ... \n");
#endif
        dm75xx_status = DM75xx_Board_PCI_Master(rtdBoard, &pci_master_status);
        DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_Board_PCI_Master");
        if (!pci_master_status) {
                error(EXIT_FAILURE, errno, "ERROR: Board is not PCI Master!");
        }
	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Main program code.
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Clear ADC FIFO
	 */
#ifdef DEBUG
        if(!noConsole)
            fprintf(stdout, "Clearing ADC FIFO ... \n");
#endif
        dm75xx_status = DM75xx_ADC_Clear(rtdBoard);
        DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_Clear_AD_FIFO");
	/*
	 * Check FIFO Status
	 */
        uint16_t data;
        dm75xx_status = DM75xx_FIFO_Get_Status(rtdBoard, &data);
        DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_FIFO_Get_Status");
#ifdef DEBUG
        if(!noConsole)
            fprintf(stdout, "FIFO Status: 0x%4x\n", data);
#endif
	/*
	 * Enable Channel Gain Table
	 */
        
#ifdef DEBUG
        if(!noConsole)
            fprintf(stdout, "Enabling Channel Gain Latch ... \n");
#endif
        dm75xx_status = DM75xx_CGT_Enable(rtdBoard, 0xFF);
        DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_CGT_Enable");
        
#ifdef DEBUG
        if(!noConsole){
            fprintf(stdout, "Writing Channel Gain Table entry ... \n");
            fprintf(stdout, "Using %d channels is: ", settings->maxChannelsNumber);
        }
#endif
        for(int ch=0; ch < settings->maxChannelsNumber; ch++){
                /*
                 * Create CGT Entry
                 */
            cgt[ch].channel = settings->phisic_chn[ch];
#ifdef DEBUG
            if(!noConsole)
                fprintf(stdout, "%d ", cgt[ch].channel);
#endif
            cgt[ch].gain = 0;
            cgt[ch].nrse = 0;
            cgt[ch].range = 0;
            cgt[ch].ground = 1;
            cgt[ch].pause = 0;
            cgt[ch].dac1 = 0;
            cgt[ch].dac2 = 0;
            cgt[ch].skip = 0;

        /*
         * Write the entry to the CGT Latch register (used for single channel)
         */
            dm75xx_status = DM75xx_CGT_Write(rtdBoard, cgt[ch]);
            DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_CGT_Latch");
        }
#ifdef DEBUG
        if(!noConsole)
            fprintf(stdout, "\n");
#endif
	/*
	 * Setup pacer clock
	 */
#ifdef DEBUG
        if(!noConsole)
            fprintf(stdout, "Setting up Pacer Clock ... \n");
#endif
        dm75xx_status = DM75xx_PCLK_Setup(rtdBoard,
                                          DM75xx_PCLK_INTERNAL,
                                          DM75xx_PCLK_FREQ_8_MHZ,
                                          DM75xx_PCLK_NO_REPEAT,
//                                          DM75xx_PCLK_START_ETRIG,
                                          DM75xx_PCLK_START_SOFTWARE,
                                          DM75xx_PCLK_STOP_SOFTWARE,
                                          (DEF_PC_RATE_ON_CHANNEL * settings->maxChannelsNumber), &actualRate);
        DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_PCLK_Setup");
#ifdef DEBUG
        if(!noConsole)
            fprintf(stdout, "%f\n", actualRate);
#endif
	/*
	 * Set ADC Conversion Signal Select
	 */
        dm75xx_status =
        DM75xx_ADC_Conv_Signal(rtdBoard, DM75xx_ADC_CONV_SIGNAL_PCLK);
        DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_ADC_Conv_Signal");
	/*
	 * Configure Port 0 Direction
	 */
#ifdef DEBUG
        if(!noConsole)
            fprintf(stdout, "Configuring Port 0 Direction ... \n");
#endif
        dm75xx_status = DM75xx_DIO_Set_Direction(rtdBoard, DM75xx_DIO_PORT0, 0x03);
        DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_DIO_Set_Direction");

#ifdef DEBUG
        if(!noConsole)
            fprintf(stdout, "Configuring Port 1 Direction ... \n");
#endif
        dm75xx_status = DM75xx_DIO_Set_Direction(rtdBoard, DM75xx_DIO_PORT1, 0xFF);
        DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_DIO_Set_Direction");

#ifdef DEBUG
        if(!noConsole)
            fprintf(stdout, "DIO Start DM75xx_DIO_Set_Port write 0x00...\n");
#endif
        dm75xx_status = DM75xx_DIO_Set_Port(rtdBoard, DM75xx_DIO_PORT1, 0x00);
        DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_DIO_Set_Port write 0x00");

#ifdef DEBUG
        if(!noConsole)
            fprintf(stdout, "DIO Start DM75xx_DIO_Set_Port write 0x00...\n");
#endif
        dm75xx_status = DM75xx_DIO_Set_Port(rtdBoard, DM75xx_DIO_PORT0, 0x00);
        DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_DIO_Set_Port write 0x00");

#ifdef DEBUG
        if(!noConsole)
            fprintf(stdout, "DIO Start DM75xx_DIO_Set_Port write 0x80...\n");
#endif
        dm75xx_status = DM75xx_DIO_Set_Port(rtdBoard, DM75xx_DIO_PORT0, 0x80);
        DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_DIO_Set_Port write 0x80");

        /*
	 * Installing User-Space ISR
	 */
#ifdef DEBUG
        if(!noConsole)
            fprintf(stdout, "Installing User-Space ISR ... \n");
#endif
        dm75xx_status = DM75xx_InstallISR(rtdBoard, dma_callback, SCHED_FIFO, 99);
        DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_InstallISR");
	/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	   Initialize and Start DMA
	   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
	/*
	 * Create user buffer for collecting data from the device
	 */
#ifdef DEBUG
        if(!noConsole)
            fprintf(stdout, "Creating User Buffer ... \n");
#endif
        dm75xx_status = DM75xx_DMA_Buffer_Create(rtdBoard,
                                                 &dmaUserBuffer,
                                                 DM75xx_DMA_CHANNEL_0,
                                                 DEF_DMA_BUFFER_SIZE * DMA_MAX_QUEUE);
        DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_DMA_Buffer_Create");
        for(int i=0; i < DMA_MAX_QUEUE; i++){
            dma_u_buffers[i] = dmaUserBuffer + (i * DEF_DMA_BUFFER_SIZE / 2);
#ifdef DEBUG
            if(!noConsole)
            fprintf(stdout, "buf %i=%x\n", i, dma_u_buffers[i]);
#endif
        }
#ifdef DEBUG
        if(!noConsole)
            fprintf(stdout, "Initializing DMA Channel 0 ... \n");
#endif
        dm75xx_status = DM75xx_DMA_Initialize(rtdBoard, DM75xx_DMA_CHANNEL_0,
                                              DM75xx_DMA_FIFO_ADC,
                                              DM75xx_DMA_DEMAND_FIFO_ADC,
                                              DEF_DMA_BUFFER_SIZE, &dmaDmaBuffer);
        DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_DMA_Initialize");
	/*
	 * Enable DMA Channel 0 Done Interrupt
	 */
#ifdef DEBUG
        if(!noConsole)
            fprintf(stdout, "Enabling DMA Channel 0 Interrupt ... \n");
#endif
        dm75xx_status = DM75xx_Interrupt_Enable(rtdBoard, DM75xx_INT_DMA_0);
        DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_Interrupt_Enable");
	/*
	 * Enabling DMA
	 */
#ifdef DEBUG
        if(!noConsole)
            fprintf(stdout, "Enabling DMA Channel 0 ... \n");
#endif
        dm75xx_status = DM75xx_DMA_Enable(rtdBoard, DM75xx_DMA_CHANNEL_0, 0xFF);
        DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_DMA_Enable");

        StartStopInductor(true);
        /*
	 * Start DMA
	 */
#ifdef DEBUG
        if(!noConsole)
            fprintf(stdout, "Starting DMA Channel 0 ... \n");
#endif
        dm75xx_status = DM75xx_DMA_Start(rtdBoard, DM75xx_DMA_CHANNEL_0);
        DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_DMA_Start");
	/*
	 *Start the pacer clock
	 */
#ifdef DEBUG
        if(!noConsole)
            fprintf(stdout, "Starting Pacer Clock ... \n");
#endif
        dm75xx_status = DM75xx_PCLK_Start(rtdBoard);
        DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_PCLK_Start");
    }
}

void Dma::StartStopInductor(bool on_off){
    /*
     * Setup User Timer/Counter 0
     */
    uint16_t divisor;
    if(on_off){
       divisor = (uint16_t) (8000000 / (DEF_UTC_RATE));
#ifdef DEBUG
       if(!noConsole)
           fprintf(stdout, "User Timer/Counter 0 Start ... with %dGz\n", 8000000/divisor );
#endif
    } else {
        divisor = 0;
#ifdef DEBUG
        if(!noConsole)
           fprintf(stdout, "User Timer/Counter 0 Stop ... with %dGz\n", 0 );
#endif
    }
           dm75xx_status = DM75xx_UTC_Setup(rtdBoard,
                                            DM75xx_UTC_0,
                                            DM75xx_CUTC_8_MHZ,
                                            DM75xx_GUTC_NOT_GATED,
                                            DM75xx_UTC_MODE_SQUARE_WAVE,
                                            divisor);
           DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_UTC_Setup");
}

void Dma::Stop(){
    /*
     * Stop pacer clock
    */
    DM75xx_PCLK_Stop(rtdBoard);
#ifdef DEBUG
    if(!noConsole)
        fprintf(stdout, "Stopping Pacer Clock... \n");
#endif
    /*
    * Disable DMA Channel 0 Done Interrupt
    */
#ifdef DEBUG
    if(!noConsole)
	fprintf(stdout, "Disabling DMA Channel 0 Interrupt ... \n");
#endif
    dm75xx_status = DM75xx_Interrupt_Disable(rtdBoard, DM75xx_INT_DMA_0);
    DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_Interrupt_Disable");
	/*
	 * Disable DMA
	 */
#ifdef DEBUG
    if(!noConsole)
	fprintf(stdout, "Disabling DMA... \n");
#endif
    dm75xx_status = DM75xx_DMA_Abort(rtdBoard, DM75xx_DMA_CHANNEL_0);
    DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_DMA_Enable");
    
    
    /*
     * Stop Inductor
     */
    DM75xx_DIO_Set_Port(rtdBoard, DM75xx_DIO_PORT1, 0);
    DM75xx_Return_Status(rtdBoard, dm75xx_status,
                         "DM75xx_DIO_Set_Port");

    StartStopInductor(false);
    /*
    * Free the user space DMA buffer
    */
#ifdef DEBUG
    if(!noConsole)
	fprintf(stdout, "Removing Collection Buffer... from: %p\n", dmaUserBuffer);
#endif
    if(dmaUserBuffer){
        dm75xx_status = DM75xx_DMA_Buffer_Free(rtdBoard,
                                               &dmaUserBuffer,
                                               DM75xx_DMA_CHANNEL_0);
        DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_DMA_Buffer_Free ");
    }
    StartStopInductor(false);
        
#ifdef DEBUG
    if(!noConsole)
	fprintf(stdout, "Removing User - Space ISR... \n");
#endif
    dm75xx_status = DM75xx_RemoveISR(rtdBoard);
    DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_RemoveISR ");
    
//#ifdef DEBUG
//    if(!noConsole)
//	fprintf(stdout, "Board Close... \n");
//#endif
//    dm75xx_status = DM75xx_Board_Close(rtdBoard);
//    DM75xx_Return_Status(rtdBoard, dm75xx_status, "DM75xx_Board_Close");
}
