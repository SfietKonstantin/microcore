/*
 * Copyright (C) 2016 Lucien XU <sfietkonstantin@free.fr>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * The names of its contributors may not be used to endorse or promote
 *     products derived from this software without specific prior written
 *     permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 */

#ifndef MICROCORE_CORE_IJOB_H
#define MICROCORE_CORE_IJOB_H

#include <memory>
#include <functional>

namespace microcore { namespace core {

/**
 * @brief A job
 *
 * This class represents a job. A job is executed via
 * execute(). The execution can be asynchronous. Two callbacks
 * have to be passed to the execute() function. One is called
 * when the job is successful and one is called when there is
 * an error.
 *
 * Result
 */
template<class Result, class Error>
class IJob
{
public:
    /**
     * @brief Type of the success callback
     */
    using OnResult_t = std::function<void (Result &&)>;
    /**
     * @brief Type of the error callback
     */
    using OnError_t = std::function<void (Error &&)>;
    /**
     * @brief Destructor
     */
    virtual ~IJob() {}
    /**
     * @brief Execute a job
     *
     * Implement this method to perform a task. Use the callbacks
     * passed as arguments to indicate if the job has succeded or
     * failed.
     *
     * @param onResult callback used to indicate if the job is successful.
     * @param onError callback used to indicate if the job has failed.
     */
    virtual void execute(OnResult_t &&onResult, OnError_t &&onError) = 0;
};

}}

#endif // MICROCORE_CORE_IJOB_H
