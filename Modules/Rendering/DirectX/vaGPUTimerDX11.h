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

#include "Rendering/vaGPUTimer.h"

#include "Rendering/DirectX/vaDirectXCore.h"
#include "Rendering/DirectX/vaDirectXTools.h"

#include <vector>

namespace VertexAsylum
{

    class vaGPUTimerDX11 : public vaGPUTimer
    {
        static const int                            c_dataQueryMaxLag = 4;                       // in frames - doesn't work if using more than number of backbuffers
        static const int                            c_historyLength = 4 + c_dataQueryMaxLag;

    protected:
        friend class vaRenderDeviceDX11;
        static void                                 OnDeviceAndContextCreated( ID3D11Device* device );
        static void                                 OnDeviceAboutToBeDestroyed( );
        static void                                 OnFrameStart( );
        static void                                 OnFrameEnd( );

    private:
        static void                                 FrameFinishQueries( bool forceAll );

        ID3D11Query *                               GetDisjointQuery( );
        void                                        ReleaseDisjointQuery( ID3D11Query * q );
        ID3D11Query *                               GetTimerQuery( );
        void                                        ReleaseTimerQuery( ID3D11Query * q );

    public:

        // NON-STATIC bit
        struct GPUTimerInfo
        {
            D3D11_QUERY_DATA_TIMESTAMP_DISJOINT      DisjointData;
            UINT64                                   StartData;
            UINT64                                   StopData;
            ID3D11Query *                            DisjointQuery;
            ID3D11Query *                            StartQuery;
            ID3D11Query *                            StopQuery;
            bool                                     QueryActive;
            double                                   TimingResult;
            __int64                                  FrameID;
        };

        int                                         m_historyLastIndex;
        GPUTimerInfo                                m_history[c_historyLength];
        bool                                        m_active;

        double                                      m_lastTime;
        double                                      m_avgTime;

        bool                                        m_orphaned;

        ID3D11DeviceContext *                       m_deviceContext;
        ID3D11DeviceContext2 *                      m_deviceContext2;       // to make things confusing, this one is reference counted; the one above (m_deviceContext) isn't

        vaGPUTimerDX11( const vaConstructorParamsBase * params );    // if deviceContext == null, use the immediate context!
        virtual                                     ~vaGPUTimerDX11( );

    public:
        virtual void                                SetAPIContext( void * gpuAPIContext );
        virtual void *                              GetAPIContext( )                        { return (void*)m_deviceContext; }

        // vaGPUTimer implementation
        virtual void                                Start( );
        virtual void                                Stop( );

        virtual double                              GetLastTime( ) const { return m_lastTime; }
        virtual double                              GetAvgTime( ) const { return m_avgTime; }

    private:
        void                                        FinishQueries( bool forceAll );
    };


}
