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

// This file is autogenerated by microgen.py

#ifndef MICROCORE_${module.upper()}_${name.upper()}_H
#define MICROCORE_${module.upper()}_${name.upper()}_H

#include <core/globals.h>
% for include in includes:
#include <${include}>
% endfor

namespace microcore { namespace ${module} {

class ${name}
{
public:
    explicit ${name}() = default;
    explicit ${name}
    (
        % for i, property in enumerate(properties):
        % if i != len(properties) - 1:
        ${property["setter_type"]}${property["name"]},
        % else:
        ${property["setter_type"]}${property["name"]}
        % endif
        %endfor
    );
    DEFAULT_COPY_DEFAULT_MOVE(${name});
    % for property in properties:
    ${property["type"]} ${property["getter"]}() const;
    % if property["access"] == "rw":
    void ${property["setter"]}(${property["setter_type"]}${property["name"]});
    % endif
    % endfor
private:
    % for property in properties:
    ${property["type"]} m_${property["name"]} {${property["initial_value"]}};
    % endfor
}

}}

#endif // MICROCORE_${module.upper()}_${name.upper()}_H