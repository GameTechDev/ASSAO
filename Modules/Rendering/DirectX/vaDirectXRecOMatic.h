///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016, Intel Corporation
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation 
// the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of 
// the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
// SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// File changes (yyyy-mm-dd)
// 2016-09-07: filip.strugar@intel.com: first commit (extracted from VertexAsylum codebase, 2006-2016 by Filip Strugar)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

/*

#include "..\Common.h"

#ifdef USE_REC_O_MATIC

#include "..\DxEventNotifier.h"

#include <string>

// this class is just horrible
class DxRecOMatic : DxEventReceiver
{
   bool                       isRecording;
   std::string                path;
   float                      frameDeltaTime;

   IDirect3DSurface9 *        screenGrabSourceSurface;
   IDirect3DSurface9 *        screenGrabLockableSurface;

   //Avi *                      avi;

   BITMAPINFO                 screenBitmapInfo;

   HBITMAP                    screenBitmap;
   void *                     screenBitmapData;
   int                        screenBitmapScanLineSize;
   HDC                        captureDC;

   CRITICAL_SECTION           CS;

   HANDLE                     workerThread;
   bool                       workerThreadHasJob;
   bool                       shutdownWorker;

   std::string                fileNameBase;
   int                        outFrameCounter;

public:
   DxRecOMatic(void);
   ~DxRecOMatic(void);

   bool                       StartRecording( const char * path, int frameRate = 25 );
   void                       StopRecording( );

   bool                       IsRecording( )                                                    { return isRecording; }
   float                      GetFrameDeltaTime( )                                              { return frameDeltaTime; }

   void                       GrabBackbuffer( );

   void                       Render( );

private:

   virtual HRESULT 				OnCreateDevice();
   virtual HRESULT            OnResetDevice(const D3DSURFACE_DESC* pBackBufferSurfaceDesc);
   virtual void               OnLostDevice();
   virtual void               OnDestroyDevice();

private:
   friend DWORD WINAPI        ThreadProcProxy( LPVOID lpThreadParameter );

   unsigned int               WriterThreadProc();
};

#endif //USE_REC_O_MATIC

*/