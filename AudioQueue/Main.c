//
//  Main.c
//  AudioQueue
//
//  Created by Rodrigo Morbach on 02/03/15.
//  Copyright (c) 2015 Rodrigo Morbach. All rights reserved.
//

#include "Header.h"



void MyAudioQueueInputCallback(
                               void                                *aqData,
                               AudioQueueRef                       inAQ,
                               AudioQueueBufferRef                 inBuffer,
                               const AudioTimeStamp                *inStartTime,
                               UInt32                              inNumPackets,
                               const AudioStreamPacketDescription  *inPacketDesc
                               )
{
    
    struct AQRecorderState * recordState = (struct AQRecorderState*)aqData;
    if (inNumPackets == 0 &&
        recordState->mDataFormat.mBytesPerPacket != 0)
        inNumPackets =
        inBuffer->mAudioDataByteSize / recordState->mDataFormat.mBytesPerPacket;
    if (!recordState->mIsRunning)
    {
        printf("Not recording, returning\n");
        return;
    }
    
    printf("Writing buffer %lld\n", recordState->mCurrentPacket);
    OSStatus status = AudioFileWritePackets(recordState->mAudioFile,
                                            false,
                                            inBuffer->mAudioDataByteSize,
                                            inPacketDesc,
                                            recordState->mCurrentPacket,
                                            &inNumPackets,
                                            inBuffer->mAudioData);
    if (status == 0)
    {
        recordState->mCurrentPacket += inNumPackets;
    }
    AudioQueueEnqueueBuffer(recordState->mQueueRef, inBuffer, 0, NULL);
    
    recordState->mainBuffer = inBuffer->mAudioData;
    recordState->callbackMethodImplementation(recordState->callbackTarget, recordState->callbackMethodSelector, (int)inBuffer->mAudioDataByteSize);
}


void HandleOutputBuffer (
                         void                *aqData,
                         AudioQueueRef       inAQ,
                         AudioQueueBufferRef outBuffer
                         ) {
    struct AQPlayerState *playState = (struct AQPlayerState *) aqData;
    
    if(!playState->mIsRunning)
    {
        printf("Not playing, returning\n");
        return;
    }
    
    printf("Enqueuing buffer %lld for playback\n", playState->mCurrentPacket);
    
    AudioStreamPacketDescription* packetDescs;
    
    
    UInt32 bytesRead;
    UInt32 numPackets = 8000;
    OSStatus status;
    
    status = AudioFileReadPacketData(playState->mAudioFile,
                                     false,
                                     &bytesRead,
                                     packetDescs,
                                     playState->mCurrentPacket,
                                     &numPackets,
                                     outBuffer->mAudioData);
    
    //Method below is deprecated
    /*status = AudioFileReadPackets(playState->mAudioFile,
                                  false,
                                  &bytesRead,
                                  packetDescs,
                                  playState->mCurrentPacket,
                                  &numPackets,
                                  outBuffer->mAudioData);*/
    
    //If there are still packets left, we are good to enqueue the buffer once again.
    if (numPackets)
    {
        outBuffer->mAudioDataByteSize = bytesRead;
        status = AudioQueueEnqueueBuffer(playState->mQueue,
                                         outBuffer,
                                         0,
                                         packetDescs);
        
        playState->mCurrentPacket += numPackets;
    }
    //Otherwise, lets just stop playing and release unused memory
    else
    {
        if (playState->mIsRunning)
        {
            AudioQueueStop(playState->mQueue, false);
            AudioFileClose(playState->mAudioFile);
            playState->mIsRunning = false;
        }
        AudioQueueFreeBuffer(playState->mQueue, outBuffer);
    }
    
}
