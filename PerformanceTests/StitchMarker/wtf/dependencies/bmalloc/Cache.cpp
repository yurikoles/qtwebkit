/*
 * Copyright (C) 2014-2017 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "Cache.h"
#include "Heap.h"
#include "Inline.h"
#include "PerProcess.h"

namespace bmalloc {

void Cache::scavenge(HeapKind heapKind)
{
    PerHeapKind<Cache>* caches = PerThread<PerHeapKind<Cache>>::getFastCase();
    if (!caches)
        return;

    caches->at(heapKind).allocator().scavenge();
    caches->at(heapKind).deallocator().scavenge();
}

Cache::Cache(HeapKind heapKind)
    : m_deallocator(PerProcess<PerHeapKind<Heap>>::get()->at(heapKind))
    , m_allocator(PerProcess<PerHeapKind<Heap>>::get()->at(heapKind), m_deallocator)
{
}

NO_INLINE void* Cache::tryAllocateSlowCaseNullCache(HeapKind heapKind, size_t size)
{
    return PerThread<PerHeapKind<Cache>>::getSlowCase()->at(heapKind).allocator().tryAllocate(size);
}

NO_INLINE void* Cache::allocateSlowCaseNullCache(HeapKind heapKind, size_t size)
{
    return PerThread<PerHeapKind<Cache>>::getSlowCase()->at(heapKind).allocator().allocate(size);
}

NO_INLINE void* Cache::allocateSlowCaseNullCache(HeapKind heapKind, size_t alignment, size_t size)
{
    return PerThread<PerHeapKind<Cache>>::getSlowCase()->at(heapKind).allocator().allocate(alignment, size);
}

NO_INLINE void Cache::deallocateSlowCaseNullCache(HeapKind heapKind, void* object)
{
    PerThread<PerHeapKind<Cache>>::getSlowCase()->at(heapKind).deallocator().deallocate(object);
}

NO_INLINE void* Cache::reallocateSlowCaseNullCache(HeapKind heapKind, void* object, size_t newSize)
{
    return PerThread<PerHeapKind<Cache>>::getSlowCase()->at(heapKind).allocator().reallocate(object, newSize);
}

} // namespace bmalloc
