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

#include "vaCoreTypes.h"

namespace VertexAsylum
{

    //   Written in 2014 by Sebastiano Vigna (vigna@acm.org)
    // (Murmurhash seed bit added by Filip Strugar and it's maybe somehow wrong)
    // 
    // To the extent possible under law, the author has dedicated all copyright
    // and related and neighboring rights to this software to the public domain
    // worldwide. This software is distributed without any warranty.
    // 
    // See <http://creativecommons.org/publicdomain/zero/1.0/>.
    // 
    // This is the fastest generator passing BigCrush without systematic
    // errors, but due to the relatively short period it is acceptable only
    // for applications with a very mild amount of parallelism; otherwise, use
    // a xorshift1024* generator.
    // 
    // The state must be seeded so that it is not everywhere zero. If you have
    // a 64-bit seed, we suggest to pass it twice through MurmurHash3's
    // avalanching function.

    class vaRandom
    {
        uint64 s[2];

    public:
        // this class can be instantiated, but a singleton is provided as a convenience for global use, when Seed/order is unimportant.
        static vaRandom     Singleton;

    public:
        vaRandom( )               { Seed( 0 ); }
        vaRandom( int seed )      { Seed( seed ); }
        ~vaRandom( )              { }

        inline void         Seed( int seed )
        {
            // A modification of https://en.wikipedia.org/wiki/MurmurHash https://sites.google.com/site/murmurhash/ by Austin Appleby in 2008,  under MIT license

            seed = seed ^ ( ( seed ^ 0x85ebca6b ) >> 13 ) * 0xc2b2ae3;
            s[0] = (uint64)seed * 0xff51afd7ed558ccd;
            s[1] = (uint64)( seed ^ 0xe6546b64 ) * 0xc4ceb9fe1a85ec53;
            NextUINT64( );
        }

        uint64              NextUINT64( )                                   { return next( ); }
        int64               NextINT64( )                                    { return (int64)next( ); }

        uint32              NextUINT32( )                                   { return (uint32)next( ); }
        int32               NextINT32( )                                    { return (int32)next( ); }

        float               NextFloat( )                                    { return NextUINT32( ) / (float)0xFFFFFFFFU; }
        float               NextFloatRange( float rmin, float rmax )        { return rmin + ( rmax - rmin ) * NextFloat( ); }

        int32               NextIntRange( int32 rmin, int32 rmax )          { assert( (rmax - rmin) >= 0 ); if( (rmax - rmin) <= 0 ) return rmin; return rmin + int32(next() % uint64( rmax - rmin )); }

    private:
        uint64 next( void )
        {
            uint64 s1 = s[0];
            const uint64 s0 = s[1];
            s[0] = s0;
            s1 ^= s1 << 23; // a
            return ( s[1] = ( s1 ^ s0 ^ ( s1 >> 17 ) ^ ( s0 >> 26 ) ) ) + s0; // b, c
        }
    };

}