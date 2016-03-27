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

namespace microcore { namespace ${module} { namespace qt {

% for nested_class in classes:
${nested_class["c_nested"]}
% endfor
${name}Object::${name}Object(QObject *parent)
    : QObject(parent)
{
    % for property in properties:
    % if property["is_object"]:
    m_${property["name"]} = new ${property["qt_class"]}(this);
    % endif
    % endfor
}

${name}Object::${name}Object(${name} &&data, QObject *parent)
    : QObject(parent), m_data(std::move(data))
{
    % for property in properties:
    % if property["is_object"]:
    m_${property["name"]} = new ${property["qt_class"]}(m_data.${property["getter"]}(), this);
    % endif
    % endfor
}

% for property in properties:
${property["qt_type"]} ${name}Object::${property["getter"]}() const
{
    % if property["is_object"]:
    return m_${property["name"]};
    % else:
    return m_data.${property["getter"]}();
    % endif
}

% endfor
const ${name} & ${name}Object::data() const
{
    return m_data;
}

void ${name}Object::update(${name} &&data)
{
% if not const:
    ${name} oldData {m_data};
    m_data = std::move(data);
    % for property in properties:
    % if property["is_object"]:
    m_${property["name"]}->update(m_data.${property["getter"]}());
    % endif
    % endfor
    % for property in properties:
    % if property["access"] != "c":
    if (oldData.${property["getter"]}() != data.${property["getter"]}()) {
        Q_EMIT ${property["name"]}Changed();
    }
    % endif
    % endfor
% else:
    Q_UNUSED(data);
% endif
}

}}}
