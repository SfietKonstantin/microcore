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

#ifndef MICROCORE_${module.upper()}_QT_${name.upper()}OBJECT_H
#define MICROCORE_${module.upper()}_QT_${name.upper()}OBJECT_H

#include <QObject>
#include "${name.lower()}.h"

namespace microcore { namespace ${module} { namespace qt {

class ${name}Object : public QObject
{
    Q_OBJECT
    % for property in properties:
    % if property["access"] == "c":
    Q_PROPERTY(${property["type"]} READ ${property["getter"]} CONSTANT)
    % elif property["access"] == "r":
    Q_PROPERTY(${property["type"]} READ ${property["getter"]} NOTIFY ${property["name"]}Changed)
    % elif property["access"] == "rw":
    Q_PROPERTY(${property["type"]} READ ${property["getter"]} WRITE ${property["setter"]} NOTIFY ${property["name"]}Changed)
    % endif
    % endfor
public:
    explicit ${name}Object(QObject *parent = nullptr);
    DISABLE_COPY_DISABLE_MOVE(${name}Object);
    % for property in properties:
    ${property["type"]} ${property["getter"]}() const;
    % endfor
    const ::microcore::${module}::${name} & data() const;
    void update(::microcore::${module}::${name} &&data);
% if not const:
Q_SIGNALS:
    % for property in properties:
    % if property["access"] != "c":
    void ${property["name"]}Changed();
    % endif
    % endfor
% endif
private:
    ::microcore::${module}::${name} m_data {};
}

}}}

#endif // MICROCORE_${module.upper()}_QT_${name.upper()}OBJECT_H
