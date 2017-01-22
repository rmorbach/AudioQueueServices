//
//  ViewController.m
//  AudioQueue
//
//  Created by Rodrigo Morbach on 27/02/15.
//  Copyright (c) 2015 Rodrigo Morbach. All rights reserved.
//

#import "ViewController.h"
#include "Header.h"
#import <AVFoundation/AVFoundation.h>

@interface ViewController ()
{
    void *mainBuffer;
    NSMutableData *d;
    unsigned int mainBufferOffset;
    CFURLRef filePath;
}

@property (nonatomic) NSMutableData * partialAudio;

@end

void * Buffer;

@implementation ViewController
{
    struct AQRecorderState adRecorder;
    struct AQPlayerState adPlayer;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    self.partialAudio = [[NSMutableData alloc]init];
    d = [[NSMutableData alloc]init];
    mainBuffer = malloc(3200);
    char path[256];
    [self getFilename:path maxLenth:sizeof path];
    filePath = CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)path, strlen(path), false);
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}
- (IBAction)record:(id)sender {
    
    [self setupAudioFormat:&adRecorder.mDataFormat];
    
    adRecorder.mCurrentPacket = 0;
    SEL selector = @selector(onRecordAudio:);
    adRecorder.callbackTarget = self;
    adRecorder.callbackMethodSelector = selector;
    adRecorder.callbackMethodImplementation = [self methodForSelector:selector];
    
    OSStatus status;
    status = AudioQueueNewInput(&adRecorder.mDataFormat,
                                MyAudioQueueInputCallback,
                                &adRecorder,
                                0,
                                kCFRunLoopCommonModes,
                                0,
                                &adRecorder.mQueueRef);
    
    UInt32 dataFormatSize = sizeof (adRecorder.mDataFormat);
    
    AudioQueueGetProperty (
                           adRecorder.mQueueRef,
                           kAudioQueueProperty_StreamDescription,
                           &adRecorder.mDataFormat,
                           &dataFormatSize
                           );
    
    if (status == 0)
    {
        // Prime recording buffers with empty data
        for (int i = 0; i < 3; i++)
        {
            AudioQueueAllocateBuffer(adRecorder.mQueueRef, 16000, &adRecorder.bufferRef[i]);
            AudioQueueEnqueueBuffer (adRecorder.mQueueRef, adRecorder.bufferRef[i], 0, NULL);
        }
        
        status = AudioFileCreateWithURL(filePath,
                                        kAudioFileAIFFType,
                                        &adRecorder.mDataFormat,
                                        kAudioFileFlags_EraseFile,
                                        &adRecorder.mAudioFile);
        if (status == 0)
        {
            adRecorder.mIsRunning = true;
            status = AudioQueueStart(adRecorder.mQueueRef, NULL);
        }
    }
    
    if (status != 0)
    {
        [self stop:nil];
        
    }
    
}
- (IBAction)stop:(UIButton *)sender {
    adRecorder.mIsRunning = false;
    
    AudioQueueStop(adRecorder.mQueueRef, true);
    for(int i = 0; i < 3; i++)
    {
        AudioQueueFreeBuffer(adRecorder.mQueueRef, adRecorder.bufferRef[i]);
    }
    
    AudioQueueDispose(adRecorder.mQueueRef, true);
    AudioFileClose(adRecorder.mAudioFile);
    
}

- (BOOL)getFilename:(char*)buffer maxLenth:(int)maxBufferLength
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,
                                                         NSUserDomainMask, YES);
    NSString* docDir = [paths objectAtIndex:0];
    
    NSString* file = [docDir stringByAppendingString:@"/recording.aif"];
    return [file getCString:buffer maxLength:maxBufferLength encoding:NSUTF8StringEncoding];
}
- (IBAction)startPlaying:(id)sender
{
    
    
    AVAudioSession *session = [AVAudioSession sharedInstance];
    [session setCategory:AVAudioSessionCategoryPlayback error:nil];
    [session setActive:YES error:nil];
    [self setupAudioFormat:&adPlayer.mDataFormat];
    adPlayer.mCurrentPacket = 0;
    OSStatus status;
    
    status = AudioFileOpenURL(filePath, kAudioFileReadPermission, kAudioFileAIFFType, &adPlayer.mAudioFile);
    if (status == 0)
    {
        status = AudioQueueNewOutput(&adPlayer.mDataFormat,
                                     HandleOutputBuffer,
                                     &adPlayer,
                                     CFRunLoopGetCurrent(),
                                     kCFRunLoopCommonModes,
                                     0,
                                     &adPlayer.mQueue);
        
        if (status == 0)
        {
            // Allocate and prime playback buffers
            adPlayer.mIsRunning = true;
            for (int i = 0; i < 3 ; i++)
            {
                OSStatus status = AudioQueueAllocateBuffer(adPlayer.mQueue, 16000, &adPlayer.mBuffers[i]);
                if(status)
                {
                    NSLog(@"Error enqueuing buffer %ld", (long)i);
                }
                    
                HandleOutputBuffer(&adPlayer, adPlayer.mQueue, adPlayer.mBuffers[i]);
            }
            
            status = AudioQueueStart(adPlayer.mQueue, NULL);
            if (status == 0)
            {
                NSLog(@"Start audio playback");
            }
        }
    }
}

- (void)onRecordAudio:(int)argument
{
    [d appendData:[NSData dataWithBytes:adRecorder.mainBuffer length:argument]];
    [self.partialAudio appendData:[NSData dataWithBytes:adRecorder.mainBuffer length:argument]];
}

- (void)setupAudioFormat:(AudioStreamBasicDescription*)format
{
    format->mSampleRate = 8000.0;
    format->mFormatID = kAudioFormatLinearPCM;
    format->mFramesPerPacket = 1;
    format->mChannelsPerFrame = 1;
    format->mBytesPerFrame = 2;
    format->mBytesPerPacket = 2;
    format->mBitsPerChannel = 16;
    format->mReserved = 0;
    format->mFormatFlags = kLinearPCMFormatFlagIsBigEndian     |
    kLinearPCMFormatFlagIsSignedInteger |
    kLinearPCMFormatFlagIsPacked;
}


@end



