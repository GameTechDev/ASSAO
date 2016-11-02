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

#include "Rendering/Effects/vaSimpleParticles.h"

#include "Rendering/vaDebugCanvas.h"


#include <functional>

using namespace VertexAsylum;

#define ALLOW_MULTITHREADED_PARTICLES

//#pragma optimize( "gxy", on )

vaSimpleParticleSystem::vaSimpleParticleSystem( ) 
{ 
    delegate_particlesTickShader.Bind( this, &vaSimpleParticleSystem::DefaultParticlesTickShader );
    delegate_drawBufferUpdateShader.Bind( this, &vaSimpleParticleSystem::DefaultDrawBufferUpdateShader );

    m_settings.MaxParticles             = 2 * 1024  * 1024;

    m_settings.FadeAlphaFrom            = 0.8f;
    m_settings.VelocityDamping          = 0.05f;
    m_settings.AngularVelocityDamping   = 0.0f;
    m_settings.Gravity                  = vaVector3( 0.0f, 0.0f, -9.81f );

    m_lastTickEmitterCount              = 0;
    m_lastTickParticleCount             = 0;
    m_debugParticleDrawCountLimit       = -1;

    //m_lastParticleID    = (uint32)-1;
    m_lastEmitterID     = (uint32)-1;

    m_sortedAfterTick = false;

    m_lastTickID = -1;

    m_boundingBox = vaBoundingBox::Degenerate;
}

void vaSimpleParticleSystem::ReleaseEmitterPtr( std::shared_ptr<vaSimpleParticleEmitter> & ptr )
{ 
    assert( ptr.unique( ) );
    if( ptr.unique( ) )  // same as ptr.use_count() == 1
    {
        m_unusedEmittersPool.push_back( ptr );
    }
    ptr = NULL;
}

void vaSimpleParticleEmitter::DefaultEmitterTickShader( vaSimpleParticleSystem & psys, vaSimpleParticleEmitter & emitter, std::vector<vaSimpleParticle> & allParticles, float deltaTime )
{
    // Warning: emitter is only allowed to push back new particles in allParticles, never remove or swap them!

    emitter.RemainingEmitterLife -= deltaTime;

    if( emitter.RemainingEmitterLife < 0 )
        return;

    emitter.TimeAccumulatedSinceLastSpawn += deltaTime;

    const int maxSpawnPerFrame = 512 * 1024;
    int numberToSpawn = vaMath::Clamp( (int)(emitter.TimeAccumulatedSinceLastSpawn * emitter.Settings.SpawnFrequencyPerSecond), 0, maxSpawnPerFrame );

    if( (numberToSpawn > 0) && (emitter.Settings.SpawnFrequencyPerSecond > 0.0f) )
        emitter.TimeAccumulatedSinceLastSpawn -= (float)numberToSpawn / emitter.Settings.SpawnFrequencyPerSecond;

    if( emitter.RemainingEmitterParticleCount != INT_MAX )
        emitter.RemainingEmitterParticleCount -= numberToSpawn;

    for( int j = 0; j < numberToSpawn; j++ )
    {
        allParticles.push_back( vaSimpleParticle() );
        vaSimpleParticle & newParticle = allParticles.back();

        emitter.LastParticleID++;
        //emitter.LastParticleID &= 0xFFFF;

        newParticle.CreationID          = emitter.LastParticleID; //(psys.LastParticleID() & 0x00FFFFFF) | ((emitter.CreationID & 0xFF) << 24);
        newParticle.LifeRemaining       = vaMath::Max( 0.0f, emitter.Settings.SpawnLife + (emitter.Settings.SpawnLifeRandomAddSub )    * vaRandom::Singleton.NextFloatRange( -1.0f, 1.0f ) );
        newParticle.LifeStart           = newParticle.LifeRemaining;

        newParticle.Size                = vaMath::Max( 0.0f, emitter.Settings.SpawnSize + (emitter.Settings.SpawnSizeRandomAddSub)    * vaRandom::Singleton.NextFloatRange( -1.0f, 1.0f ) );
        newParticle.SizeChange          = emitter.Settings.SpawnSizeChange + (emitter.Settings.SpawnSizeChangeRandomAddSub)           * vaRandom::Singleton.NextFloatRange( -1.0f, 1.0f );

        newParticle.Angle               = emitter.Settings.SpawnAngle + (emitter.Settings.SpawnAngleRandomAddSub)                     * vaRandom::Singleton.NextFloatRange( -1.0f, 1.0f );
        newParticle.AngularVelocity     = emitter.Settings.SpawnAngularVelocity + (emitter.Settings.SpawnAngularVelocityRandomAddSub) * vaRandom::Singleton.NextFloatRange( -1.0f, 1.0f );

        newParticle.AffectedByGravityK  = emitter.Settings.SpawnAffectedByGravityK;
        newParticle.AffectedByWindK     = emitter.Settings.SpawnAffectedByWindK;

        if( emitter.Settings.SpawnAreaType == vaSimpleParticleEmitter::SAT_BoundingBox )
        {
            newParticle.Position        = emitter.Settings.SpawnAreaBoundingBox.RandomPointInside( vaRandom::Singleton );
        }
        else if( emitter.Settings.SpawnAreaType == vaSimpleParticleEmitter::SAT_BoundingSphere )
        {
            newParticle.Position        = emitter.Settings.SpawnAreaBoundingSphere.RandomPointInside( vaRandom::Singleton );
        }
        else
        {
            assert( false );
        }
        newParticle.Velocity        = emitter.Settings.SpawnVelocity + emitter.Settings.SpawnVelocityRandomAddSub * ( vaVector3::Random() * 2.0f - 1.0f );
        newParticle.Color           = emitter.Settings.SpawnColor + emitter.Settings.SpawnColorRandomAddSub * ( vaVector4::Random()  * 2.0f - 1.0f );
    }
}

void vaSimpleParticleSystem::DefaultParticlesTickShader( vaSimpleParticleSystem & psys, std::vector<vaSimpleParticle> & allParticles, float deltaTime )
{
    // Warning: particle shader is NOT allowed to push back new particles to allParticles, remove them or reorder them!
    if( deltaTime == 0.0f )
        return;
    
    const int loopLength = (int)allParticles.size( );
    if( loopLength == 0 )
        return;

#ifdef ALLOW_MULTITHREADED_PARTICLES
    // split evenly across all available threads
    //std::function< void( int ) > functions[vaThreadPool::c_maxPossibleThreads];
    int jobThreadCount = (int)vaMath::Min( vaThreadPool::GetInstance().GetThreadCount() * 2, vaThreadPool::c_maxPossibleThreads );

    const int perThreadBlockSize = (loopLength+jobThreadCount-1) / jobThreadCount;

    for( int j = 0; j < jobThreadCount; j++ )
    {
        //functions[j] = [&perThreadBlockSize, &loopLength, &psys, &allParticles, &deltaTime, this] ( int threadIndex ) 
        std::function< void( int ) > doChunk = [&perThreadBlockSize, &loopLength, &psys, &allParticles, &deltaTime, this] ( int threadIndex )
        { 
            const int stopAt = vaMath::Min( (threadIndex+1) * perThreadBlockSize, loopLength );
            for( int i = threadIndex * perThreadBlockSize; i < stopAt; i++ )
#else
            for( int i = 0; i < loopLength; i++ )
#endif
            {
                vaSimpleParticle & particle = allParticles[i];

                particle.LifeRemaining -= deltaTime;

                particle.Position += particle.Velocity * deltaTime;
                particle.Angle = vaMath::AngleWrap( particle.Angle + particle.AngularVelocity * deltaTime );
                particle.Size = vaMath::Max( 0.0f, particle.Size + particle.SizeChange * deltaTime );

                particle.Velocity += particle.AffectedByGravityK * m_settings.Gravity * deltaTime;
                particle.Velocity = vaMath::Lerp( particle.Velocity, vaVector3( 0.0f, 0.0f, 0.0f ), vaMath::TimeIndependentLerpF( deltaTime, m_settings.VelocityDamping ) );

                particle.AngularVelocity = vaMath::Lerp( particle.AngularVelocity, 0.0f, vaMath::TimeIndependentLerpF( deltaTime, m_settings.AngularVelocityDamping ) );
            }
#ifdef ALLOW_MULTITHREADED_PARTICLES
        };

        vaThreadPool::GetInstance().AddJob( doChunk, j );
    }

    // finish all
    vaThreadPool::GetInstance().WaitFinishAllTasks( true );
    //vaThreadPool::GetInstance().ExecuteArrayAndWait( functions, jobThreadCount );
#endif
}

void vaSimpleParticleSystem::Sort( const vaVector3 & _cameraPos, bool backToFront )
{
    m_sortedAfterTick = true;

    vaVector3 cameraPos = _cameraPos;

    const std::vector<vaSimpleParticle> & allParticles = m_particles;
    std::vector< float >  & sortValues = m_particleSortValueCache;
    std::vector< int > & sortedIndices = m_particleSortedIndices;

    if( allParticles.size( ) == 0 )
        return;

    if( allParticles.size() > sortValues.size() )
        sortValues.resize( allParticles.size( ) );
    if( m_particles.size( ) > m_particleSortedIndices.size( ) )
        m_particleSortedIndices.resize( m_particles.size( ) );

    // to disable sort
#if 0
    {
        for( size_t i = 0; i < allParticles.size( ); i++ )
        {
            sortedIndices[i] = (int)i;
        }
        m_sortedAfterTick = true;
        return;
    }
#endif


    float backToFrontMultiplier = ( backToFront ) ? ( 1.0f ) : ( -1.0f );

    struct MySortComparerBackToFront
    {
        const std::vector< float > & SortValues;
        MySortComparerBackToFront( const std::vector< float > & sortValues ) : SortValues( sortValues ) { }
        bool operator() ( int i, int j ) { return ( SortValues[i] > SortValues[j] ); }
    } mySortComparerBackToFront( sortValues );

    const int totalParticleCount = (int)allParticles.size();

    const int recursionDepth    = 5;
    const int leafBucketCount   = 1 << recursionDepth;
    int bucketSize              = ( totalParticleCount + leafBucketCount - 1 ) / leafBucketCount;
    int     sortBucketBoundaries[leafBucketCount + 1];
    int32   readinessFlags[ leafBucketCount * recursionDepth ];

    assert( vaMath::IsPowOf2(leafBucketCount) );

    for( int bucketIndex = 0; bucketIndex <= leafBucketCount; bucketIndex++ )
    {
        sortBucketBoundaries[ bucketIndex ] = vaMath::Min( ( bucketIndex ) * bucketSize, totalParticleCount );
    }
    for( int level = 0; level < recursionDepth; level++ )
        for( int bucketIndex = 0; bucketIndex < leafBucketCount; bucketIndex++ )
        {
            readinessFlags[ level * leafBucketCount + bucketIndex ] = 0;
        }

    std::function< void( int, int ) > mergeArea = [&]( int level, int bucketID )
    {
        int currentStep = 1 << level;
        int currentCount = leafBucketCount / currentStep;

        int left        = bucketID * currentStep;
        int middle      = left + currentStep/2;
        int right       = left + currentStep;
        int indexLeft   = sortBucketBoundaries[left];
        int indexMiddle = sortBucketBoundaries[middle];
        int indexRight  = sortBucketBoundaries[right];

        std::inplace_merge( sortedIndices.begin( ) + indexLeft, sortedIndices.begin( ) + indexMiddle, sortedIndices.begin( ) + indexRight, mySortComparerBackToFront );

        if( level == recursionDepth )
            return;

        if( vaThread::Interlocked_Increment( &readinessFlags[level * leafBucketCount + bucketID / 2] ) == 2 )
        {
            //int level = 0;
            mergeArea( level+1, bucketID / 2 );
        }

    };

    //std::function< void( int ) > functions[vaThreadPool::c_maxPossibleThreads];
    for( int t = 0; t < leafBucketCount; t++ )
    {
        //functions[t] = [&sortBucketBoundaries, &totalParticleCount, &bucketSize, &allParticles, &cameraPos, &backToFrontMultiplier, &sortValues, &sortedIndices, &mySortComparerBackToFront, this]( int bucketIndex )
        std::function< void( int ) > doChunk = [&]( int bucketIndex )
        {
            const int startFrom = sortBucketBoundaries[ bucketIndex ];
            const int stopAt    = sortBucketBoundaries[ bucketIndex+1 ];
            for( int i = startFrom; i < stopAt; i++ )
            {
                sortValues[i] = backToFrontMultiplier * ( allParticles[i].Position - cameraPos ).Length( );
                sortedIndices[i] = (int)i;
            }
            std::sort( sortedIndices.begin( ) + startFrom, sortedIndices.begin( ) + stopAt, mySortComparerBackToFront );

            int level = 0;
            if( vaThread::Interlocked_Increment( &readinessFlags[ level * leafBucketCount + bucketIndex / 2 ] ) == 2 )
            {
                //int level = 0;
#ifdef ALLOW_MULTITHREADED_PARTICLES
                vaThreadPool::GetInstance().AddJob( mergeArea, 1, bucketIndex / 2 );
#else
                mergeArea( 1, bucketIndex / 2 );
#endif
            }
        };
#ifdef ALLOW_MULTITHREADED_PARTICLES
        vaThreadPool::GetInstance( ).AddJob( doChunk, t );
#else
        doChunk( t );
#endif
    }

#ifdef ALLOW_MULTITHREADED_PARTICLES
    vaThreadPool::GetInstance( ).WaitFinishAllTasks( true );
#endif

    // int currentStep = 2;
    // int currentCount = leafBucketCount / currentStep;
    // 
    // // merge - unfortunately this isn't multi-threaded but it could be!
    // while( currentCount > 0 )
    // {
    //     for( int i = 0; i < currentCount; i++ )
    //         std::inplace_merge( sortedIndices.begin( ) + sortBucketBoundaries[i * currentStep], sortedIndices.begin( ) + sortBucketBoundaries[i * currentStep + currentStep/2], sortedIndices.begin( ) + sortBucketBoundaries[(i+1) * currentStep], mySortComparerBackToFront );
    // 
    //     currentStep *= 2;
    //     currentCount /= 2;
    // }

    // just plain old sort for testing the correctness of above
#if 0
    std::vector< int > testParticleSortedIndices;
    testParticleSortedIndices.resize( m_particles.size( ) );
    {
        std::vector< int > & sortedIndices = testParticleSortedIndices;

        //float distMax = FLT_MIN;
        //float distMin = FLT_MAX;
        for( size_t i = 0; i < allParticles.size( ); i++ )
        {
            sortValues[i] = backToFrontMultiplier * ( allParticles[i].Position - cameraPos ).Length( );
            sortedIndices[i] = (int)i;
            //distMax = vaMath::Max( distMax, sortValues[i] );
            //distMin = vaMath::Min( distMin, sortValues[i] );
        }

        std::sort( sortedIndices.begin( ), sortedIndices.begin() + allParticles.size(), mySortComparerBackToFront );
        m_sortedAfterTick = true;
    }
    for( size_t i = 0; i < testParticleSortedIndices.size(); i++ )
    {
        if( m_particleSortedIndices[i] != testParticleSortedIndices[i] )
        {
            float l = sortValues[m_particleSortedIndices[i]];
            float r = sortValues[testParticleSortedIndices[i]];
            assert( l == r );
        }
    }
#endif
}


void vaSimpleParticleSystem::DefaultDrawBufferUpdateShader( const vaSimpleParticleSystem & psys, const std::vector<vaSimpleParticle> & allParticles, const std::vector< int > & sortedIndices, void * outDestinationBuffer, int inDestinationBufferSize )
{
    vaBillboardSprite * particleBuffer = (vaBillboardSprite *)outDestinationBuffer;

    if( inDestinationBufferSize != sizeof(vaBillboardSprite) * allParticles.size() )
    {
        // buffer size mismatch? something is wrong
        assert( false );
        return;
    }
    
    const int loopLength = (int)allParticles.size( );
    if( loopLength == 0 )
        return;

#ifdef ALLOW_MULTITHREADED_PARTICLES
    // split evenly across all available threads
    //std::function< void( int ) > functions[vaThreadPool::c_maxPossibleThreads];
    int jobThreadCount = (int)vaMath::Min( vaThreadPool::GetInstance().GetThreadCount() * 2, vaThreadPool::c_maxPossibleThreads );

    const int perThreadBlockSize = (loopLength+jobThreadCount-1) / jobThreadCount;

    for( int j = 0; j < jobThreadCount; j++ )
    {
        //functions[j] = [&perThreadBlockSize, &loopLength, &psys, &allParticles, &sortedIndices, particleBuffer, this] ( int threadIndex ) 
        std::function< void( int ) > doChunk = [&perThreadBlockSize, &loopLength, &psys, &allParticles, &sortedIndices, particleBuffer, this] ( int threadIndex ) 
        { 
            const int stopAt = vaMath::Min( (threadIndex+1) * perThreadBlockSize, loopLength );
            for( int i = threadIndex * perThreadBlockSize; i < stopAt; i++ )
#else
            for( int i = 0; i < loopLength; i++ )
#endif
            {
                const vaSimpleParticle & particle = allParticles[sortedIndices[i]];

                vaBillboardSprite & outVertex = particleBuffer[i];

                outVertex.Position_CreationID = vaVector4( particle.Position, *((float*)&particle.CreationID) );

                vaVector4 color = particle.Color;

                color.w *= vaMath::Saturate( 1.0f - ( ( 1.0f - particle.LifeRemaining / particle.LifeStart ) - m_settings.FadeAlphaFrom ) / ( 1.0f - m_settings.FadeAlphaFrom ) );

                outVertex.Color = vaVector4::ToRGBA( color );

                float ca = vaMath::Cos( particle.Angle );
                float sa = vaMath::Sin( particle.Angle );

                outVertex.Transform2D.x =  ca * particle.Size;
                outVertex.Transform2D.y = -sa * particle.Size;
                outVertex.Transform2D.z =  sa * particle.Size;
                outVertex.Transform2D.w =  ca * particle.Size;
            }
#ifdef ALLOW_MULTITHREADED_PARTICLES
        };
        vaThreadPool::GetInstance( ).AddJob( doChunk, j );
    }

    // finish all
    vaThreadPool::GetInstance( ).WaitFinishAllTasks( true );
    //vaThreadPool::GetInstance().ExecuteArrayAndWait( functions, jobThreadCount );
#endif
}

void vaSimpleParticleSystem::DrawDebugBoxes( )
{
    vaDebugCanvas3DBase * canvas3D = vaDebugCanvas3DBase::GetInstancePtr( );

    //canvas3D->DrawBox( m_boundingBox, 0x80000000, 0, &this->GetTransform() );

    bool dbgShowEmitters = true;
    bool dbgShowParticles = false;
    if( dbgShowEmitters )
    {
        for( uint32 i = 0; i < m_emitters.size( ); i++ )
        {
            if( m_emitters[i]->Settings.SpawnAreaType == vaSimpleParticleEmitter::SAT_BoundingBox )
            {
                vaOrientedBoundingBox obb = vaOrientedBoundingBox::Transform( m_emitters[i]->Settings.SpawnAreaBoundingBox, this->GetTransform( ) );
                canvas3D->DrawBox( obb, 0x80000000, 0x08FF0000 );
            }
            else if( m_emitters[i]->Settings.SpawnAreaType == vaSimpleParticleEmitter::SAT_BoundingSphere )
            {
                vaBoundingSphere bs = m_emitters[i]->Settings.SpawnAreaBoundingSphere;
                bs.Center = vaVector3::TransformCoord( bs.Center, this->GetTransform( ) );
                canvas3D->DrawSphere( bs, 0x80000000, 0x08FF0000 );
            }
        }
    }
    if( dbgShowParticles )
    {
        for( uint32 i = 0; i < m_particles.size( ); i++ )
        {
            vaVector3 halfSize = vaVector3( m_particles[i].Size * 0.5f, m_particles[i].Size * 0.5f, m_particles[i].Size * 0.5f );
            vaBoundingBox aabb( m_particles[i].Position - halfSize, halfSize * 2 );
            vaOrientedBoundingBox obb( aabb, this->GetTransform( ) );
            canvas3D->DrawBox( aabb, 0x00000000, vaVector4::ToBGRA( m_particles[i].Color ) );
        }
    }
}

void vaSimpleParticleSystem::Tick( float deltaTime )
{
    m_lastTickID++;

    const size_t particlesBeforeEmitterShaders = m_particles.size( );
    for( size_t i = 0; i < m_emitters.size( ); i++ )
    {
        vaSimpleParticleEmitter & emitter = *m_emitters[i].get( );

        if( !m_emitters[i]->Active )
        {
            // if emitter inactive and unreferenced, reclaim the ptr!
            if( m_emitters[i].unique( ) )
            {
                ReleaseEmitterPtr( m_emitters[i] ); // WARNING: this makes "emitter" variable a zero pointer from now on!!
                if( m_emitters.size( ) == (i-1) )
                {
                    m_emitters.pop_back( );
                }
                else
                {
                    m_emitters[i] = m_emitters.back( );
                    m_emitters.pop_back( );
                    i--;
                }
            }
            continue;
        }

        if( m_emitters[i]->delegate_emitterTickShader.empty() )
            continue;

        // emitter is only allowed to push back new particles, never remove or swap them!
        m_emitters[i]->delegate_emitterTickShader( *this, *m_emitters[i].get(), m_particles, deltaTime );

        // emitter no longer active?
        if( ( ( emitter.RemainingEmitterLife <= 0 ) || ( emitter.RemainingEmitterParticleCount <= 0 ) ) )
        {
            emitter.Active = false;
            continue;
        }
    }

    const size_t particlesBeforeParticleShader = m_particles.size( );
    if( !delegate_particlesTickShader.empty( ) )
        delegate_particlesTickShader( *this, m_particles, deltaTime );
    assert( particlesBeforeParticleShader == m_particles.size( ) );


    // update bounding box and remove dead particles
    {
        m_boundingBox = vaBoundingBox::Degenerate;

        if( m_particles.size( ) > 0 )
        {
            vaVector3 bsize = vaVector3( m_particles[0].Size, m_particles[0].Size, m_particles[0].Size );
            m_boundingBox = vaBoundingBox( m_particles[0].Position - bsize * 0.5f, bsize );
        }

        for( int i = (int)m_particles.size( )-1; i >= 0; i-- )
        {
            vaSimpleParticle & particle = m_particles[i];

            if( particle.LifeRemaining < 0 )
            {
                if( (m_particles.size( )-1) == i )
                {
                    m_particles.pop_back( );
                    return;
                }
                else
                {
                    m_particles[i] = m_particles.back( );
                    m_particles.pop_back( );
                    i--;
                    continue;
                }
            }

            vaVector3 bsize = vaVector3( m_particles[i].Size, m_particles[i].Size, m_particles[i].Size );
            m_boundingBox = vaBoundingBox::Combine( m_boundingBox, vaBoundingBox( m_particles[i].Position - bsize * 0.5f, bsize ) );
        }
    }

    //// load texture if needed (this should go elsewhere to be honest)
    //if( (m_texture == NULL) && (m_settings.TextureFilePath != L"") )
    //{
    //    m_texture = vaTextureLibrary::GetInstance().Load( m_settings.TextureFilePath, false );
    //}

    m_lastTickEmitterCount  = (int)m_emitters.size();
    m_lastTickParticleCount = (int)m_particles.size();

    m_sortedAfterTick = false;
}

//#pragma optimize( "", on )

// #ifdef VA_ANT_TWEAK_BAR_ENABLED   
// void vaSimpleParticleSystem::OnAntTweakBarInitialized( TwBar * mainBar )
// {
// #ifdef VSM_SAMPLE
//     // no need for terrain settings for VSM sample
//     return;
// #endif
// 
//     if( mainBar == NULL )
//         return;
// 
//     TwAddVarRO( mainBar, "ParticlSystem::m_lastTickEmitterCount", TW_TYPE_INT32, &m_lastTickEmitterCount, "Label='Number of particle emitters' Group='ParticleSystem'" );
//     TwAddVarRO( mainBar, "ParticlSystem::m_lastTickParticleCount", TW_TYPE_INT32, &m_lastTickParticleCount, "Label='Number of particles' Group='ParticleSystem'" );
//     TwAddVarRW( mainBar, "ParticlSystem::m_debugParticleDrawCountLimit", TW_TYPE_INT32, &m_debugParticleDrawCountLimit, "Label='Debug particle count limit' Group='ParticleSystem' min=-1 max=262144 " );
//     
// }
// #endif
