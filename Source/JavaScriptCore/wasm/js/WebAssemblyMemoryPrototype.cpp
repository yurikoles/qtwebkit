/*
 * Copyright (C) 2016-2017 Apple Inc. All rights reserved.
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

#include "config.h"
#include "WebAssemblyMemoryPrototype.h"

#if ENABLE(WEBASSEMBLY)

#include "FunctionPrototype.h"
#include "JSArrayBuffer.h"
#include "JSCInlines.h"
#include "JSWebAssemblyMemory.h"
#include "JSWebAssemblyHelpers.h"

namespace JSC {
static EncodedJSValue JSC_HOST_CALL webAssemblyMemoryProtoFuncGrow(JSGlobalObject*, CallFrame*);
static EncodedJSValue JSC_HOST_CALL webAssemblyMemoryProtoFuncBuffer(JSGlobalObject*, CallFrame*);
}

#include "WebAssemblyMemoryPrototype.lut.h"


namespace JSC {

const ClassInfo WebAssemblyMemoryPrototype::s_info = { "WebAssembly.Memory", &Base::s_info, &prototypeTableWebAssemblyMemory, nullptr, CREATE_METHOD_TABLE(WebAssemblyMemoryPrototype) };

/* Source for WebAssemblyMemoryPrototype.lut.h
@begin prototypeTableWebAssemblyMemory
 grow   webAssemblyMemoryProtoFuncGrow   Function 1
 buffer webAssemblyMemoryProtoFuncBuffer Accessor 0
@end
*/

ALWAYS_INLINE JSWebAssemblyMemory* getMemory(ExecState* exec, VM& vm, JSValue value)
{
    auto throwScope = DECLARE_THROW_SCOPE(vm);

    JSWebAssemblyMemory* memory = jsDynamicCast<JSWebAssemblyMemory*>(vm, value); 
    if (!memory) {
        throwException(exec, throwScope, 
            createTypeError(exec, "WebAssembly.Memory.prototype.buffer getter called with non WebAssembly.Memory |this| value"_s));
        return nullptr;
    }
    return memory;
}

EncodedJSValue JSC_HOST_CALL webAssemblyMemoryProtoFuncGrow(JSGlobalObject* globalObject, CallFrame* callFrame)
{
    VM& vm = globalObject->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);

    JSWebAssemblyMemory* memory = getMemory(callFrame, vm, callFrame->thisValue()); 
    RETURN_IF_EXCEPTION(throwScope, { });
    
    uint32_t delta = toNonWrappingUint32(callFrame, callFrame->argument(0));
    RETURN_IF_EXCEPTION(throwScope, { });

    Wasm::PageCount result = memory->grow(vm, callFrame, delta);
    RETURN_IF_EXCEPTION(throwScope, { });

    return JSValue::encode(jsNumber(result.pageCount()));
}

EncodedJSValue JSC_HOST_CALL webAssemblyMemoryProtoFuncBuffer(JSGlobalObject* globalObject, CallFrame* callFrame)
{
    VM& vm = globalObject->vm();
    auto throwScope = DECLARE_THROW_SCOPE(vm);

    JSWebAssemblyMemory* memory = getMemory(callFrame, vm, callFrame->thisValue()); 
    RETURN_IF_EXCEPTION(throwScope, { });
    return JSValue::encode(memory->buffer(globalObject->vm(), globalObject));
}

WebAssemblyMemoryPrototype* WebAssemblyMemoryPrototype::create(VM& vm, JSGlobalObject*, Structure* structure)
{
    auto* object = new (NotNull, allocateCell<WebAssemblyMemoryPrototype>(vm.heap)) WebAssemblyMemoryPrototype(vm, structure);
    object->finishCreation(vm);
    return object;
}

Structure* WebAssemblyMemoryPrototype::createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
{
    return Structure::create(vm, globalObject, prototype, TypeInfo(ObjectType, StructureFlags), info());
}

void WebAssemblyMemoryPrototype::finishCreation(VM& vm)
{
    Base::finishCreation(vm);
    ASSERT(inherits(vm, info()));
}

WebAssemblyMemoryPrototype::WebAssemblyMemoryPrototype(VM& vm, Structure* structure)
    : Base(vm, structure)
{
}

} // namespace JSC

#endif // ENABLE(WEBASSEMBLY)
