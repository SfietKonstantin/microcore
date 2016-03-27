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

#include "${outfile}.h"

namespace microcore { namespace ${module} {

% for nested_class in classes:
${nested_class["c_nested"]}
% endfor
${name}::${name}
(
    % for i, property in enumerate(properties):
    % if i != len(properties) - 1:
    ${property["setter_type"]}${property["name"]},
    % else:
    ${property["setter_type"]}${property["name"]}
    % endif
    % endfor
)
    % for i, property in enumerate(properties):
    % if i == 0:
    : m_${property["name"]} {${property["setter_impl"]}}
    % else:
    , m_${property["name"]} {${property["setter_impl"]}}
    % endif
    % endfor
{
}

% for property in properties:
${property["nested_type"]} ${name}::${property["getter"]}() const
{
    return m_${property["name"]};
}

% if property["access"] == "rw":
void ${name}::${property["setter"]}(${property["setter_type"]}${property["name"]})
{
    m_${property["name"]} = ${property["setter_impl"]};
}

% endif
% endfor
}}
