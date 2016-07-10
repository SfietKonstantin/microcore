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
#include <QJsonObject>
#include <QJsonValue>
% for include in includes:
#include <${include}>
% endfor

using namespace ::microcore::core;
using namespace ::microcore::error;

namespace microcore { namespace ${module} {

class ${name}RequestJob final : public ${name}Job
{
public:
% for nested_class in classes:
${nested_class["c_nested"]}
% endfor
    explicit ${name}RequestJob(${name}Request &&request)
        : m_request {std::move(request)}
    {
    }
    void execute(OnResult &&onResult, OnError &&onError) override
    {
        QJsonObject root {m_request.object()};
        % for object in json_tree:
        QJsonObject ${"__".join(object[0]) + "__" + object[1]} {${"__".join(object[0])}.value(QLatin1String("${object[1]}")).toObject()};
        % endfor
        % for property in properties:
        % if not "json_optional" in property or not property["json_optional"]:
        if (!${"__".join(property["json_prefix"])}.contains(QLatin1String("${property["json_suffix"]}"))) {
            onError(Error("${module}_${name.lower()}",
                          QLatin1String("${property["json_path"]} cannot be found"),
                          m_request.toJson(QJsonDocument::Compact)));
            return;
        }
        % endif
        % endfor
        % for property in properties:
        % if property["json_type"] == "array" or property["json_type"] == "objectarray":
        std::vector<${property["nested_type"]}> ${"__".join(property["json_prefix"]) + "__" + property["json_suffix"]} {};
        % elif property["json_type"] == "object":
        ${property["nested_type"]} ${"__".join(property["json_prefix"]) + "__" + property["json_suffix"]} {};
        % endif
        % endfor
        % if json_has_complex:
        try {
            % for property in properties:
            % if property["json_type"] == "array" or property["json_type"] == "objectarray":
            for (const QJsonValue &value : ${"__".join(property["json_prefix"])}.value(QLatin1String("${property["json_suffix"]}")).toArray()) {
                % if property["json_type"] == "objectarray":
                ${"__".join(property["json_prefix"]) + "__" + property["json_suffix"]}.push_back(${property["type"]}Factory::create(value.${property["json_conversion_method"]}(), *this));
                % else:
                ${"__".join(property["json_prefix"]) + "__" + property["json_suffix"]}.push_back(value.${property["json_conversion_method"]}());
                % endif
            }
            % elif property["json_type"] == "object":
            ${"__".join(property["json_prefix"]) + "__" + property["json_suffix"]} = ${property["type"]}Factory::create(${"__".join(property["json_prefix"])}.value(QLatin1String("${property["json_suffix"]}")).${property["json_conversion_method"]}(), *this);
            % endif
            % endfor
        } catch (const std::string &error) {
            onError(Error("${module}_${name.lower()}",
                          QString::fromStdString(error),
                          m_request.toJson(QJsonDocument::Compact)));
            return;
        }
        % endif

        onResult(${name}Result(
            % for i, property in enumerate(properties):
            % if i != len(properties) - 1:
            % if property["json_type"] != "simple":
            std::move(${"__".join(property["json_prefix"]) + "__" + property["json_suffix"]}),
            % else:
            ${"__".join(property["json_prefix"])}.value(QLatin1String("${property["json_suffix"]}")).${property["json_conversion_method"]}(),
            % endif
            % else:
            % if property["json_type"] != "simple":
            std::move(${"__".join(property["json_prefix"]) + "__" + property["json_suffix"]})
            % else:
            ${"__".join(property["json_prefix"])}.value(QLatin1String("${property["json_suffix"]}")).${property["json_conversion_method"]}()
            % endif
            % endif
            % endfor
        ));
    }
private:
    ${name}Request m_request {};
};

std::unique_ptr<${name}Job> ${name}RequestFactory::create(${name}Request &&request) const
{
    return std::unique_ptr<${name}Job>(new ${name}RequestJob(std::move(request)));
}

}}
