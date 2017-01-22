//
//  Header.h
//  AudioQueue
//
//  Created by Rodrigo Morbach on 02/03/15.
//  Copyright (c) 2015 Rodrigo Morbach. All rights reserved.
//
//

#ifndef AudioQueue_Header_h
#define AudioQueue_Header_h

#include <AudioToolbox/AudioToolbox.h>
#include <objc/objc.h>



static const int kNumberofBuffers = 3;

//Struct used to record audio
struct AQRecorderState{
    AudioStreamBasicDescription  mDataFormat;
    AudioQueueBufferRef bufferRef[kNumberofBuffers];
    AudioQueueRef mQueueRef;
    AudioFileID mAudioFile;
    UInt32 bufferBytesSize;
    SInt64 mCurrentPacket;
    bool mIsRunning;
    void                         *mainBuffer;
    unsigned int                 *mainBufferOffset;
    IMP                          callbackMethodImplementation;
    __unsafe_unretained  id      callbackTarget;
    SEL                          callbackMethodSelector;
};


//Struct used to play audio
struct AQPlayerState {
    AudioStreamBasicDescription   mDataFormat;
    AudioQueueRef                 mQueue;
    AudioQueueBufferRef           mBuffers[kNumberofBuffers];
    AudioFileID                   mAudioFile;
    UInt32                        bufferByteSize;
    SInt64                        mCurrentPacket;
    UInt32                        mNumPacketsToRead;
    AudioStreamPacketDescription  *mPacketDescs;
    bool                          mIsRunning;
};

//Callback method definition to record audio
//Definition according to Apple documentation

/*!
 @typedef    AudioQueueInputCallback
 @abstract   Defines a pointer to a callback function that is called when a recording audio
 queue has finished filling a buffer.
 @discussion
 You specify a recording buffer callback when calling AudioQueueNewInput. Your callback
 is invoked each time the recording audio queue has filled a buffer with input data.
 Typically, your callback should write the audio queue buffer's data to a file or other
 buffer, and then re-queue the audio queue buffer to receive more data.
 
 @param      inUserData
 The value you've specified in the inUserData parameter of the AudioQueueNewInput
 function.
 @param      inAQ
 The audio queue that invoked the callback.
 @param      inBuffer
 An audio queue buffer, newly filled by the audio queue, containing the new audio data
 your callback needs to write.
 @param      inStartTime
 A pointer to an audio time stamp structure corresponding to the first sample contained
 in the buffer. This contains the sample time of the first sample in the buffer.
 @param      inNumberPacketDescriptions
 The number of audio packets contained in the data provided to the callback
 @param      inPacketDescs
 For compressed formats which require packet descriptions, the packet descriptions
 produced by the encoder for the incoming buffer.
 */

void MyAudioQueueInputCallback(
                                      void                                *aqData,
                                      AudioQueueRef                       inAQ,
                                      AudioQueueBufferRef                 inBuffer,
                                      const AudioTimeStamp                *inStartTime,
                                      UInt32                              inNumPackets,
                                      const AudioStreamPacketDescription  *inPacketDesc
                                      );

//Callback method definition to play audio
//Definition according to Apple documentation
/*!
 @typedef    AudioQueueOutputCallback
 @abstract   Defines a pointer to a callback function that is called when a playback audio
 queue has finished taking data from a buffer.
 
 @discussion
 A playback buffer callback is invoked when the audio queue has finished with the data to
 be played and the buffer is available to your application for reuse. Your application
 might want to immediately refill and re-enqueue the completed buffer at this time.
 @param      inUserData
 The value specified by the inUserData parameter of the AudioQueueNewOutput function.
 @param      inAQ
 The audio queue that invoked the callback.
 @param      inBuffer
 The audio queue buffer made available by the audio queue.
 */
void HandleOutputBuffer (
                                void                 *aqData,
                                AudioQueueRef        inAQ,
                                AudioQueueBufferRef  inBuffer
);
#endif
