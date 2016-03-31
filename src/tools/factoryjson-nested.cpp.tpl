class ${name}Factory
{
public:
% for nested_class in classes:
${nested_class["c_nested"]}
% endfor
    static ${nested_name} create(const QJsonObject &root, ${root_name}RequestJob &parent)
    {
        % if len(classes) == 0:
        Q_UNUSED(parent)
        % endif
        % for object in json_tree:
        QJsonObject ${"__".join(object[0]) + "__" + object[1]} {${"__".join(object[0])}.value(QLatin1String("${object[1]}")).toObject()};
        % endfor
        % for property in properties:
        % if not "json_optional" in property or not property["json_optional"]:
        if (!${"__".join(property["json_prefix"])}.contains(QLatin1String("${property["json_suffix"]}"))) {
            throw std::string("In ${nested_name}, ${property["json_path"]} cannot be found");
        }
        % endif
        % endfor
        % for property in properties:
        % if property["json_type"] == "array" or property["json_type"] == "objectarray":
        std::vector<${property["nested_type"]}> ${"__".join(property["json_prefix"]) + "__" + property["json_suffix"]} {};
        for (const QJsonValue &value : ${"__".join(property["json_prefix"])}.value(QLatin1String("${property["json_suffix"]}")).toArray()) {
            % if property["json_type"] == "objectarray":
            ${"__".join(property["json_prefix"]) + "__" + property["json_suffix"]}.push_back(${property["type"]}Factory::create(value.${property["json_conversion_method"]}(), parent));
            % else:
            ${"__".join(property["json_prefix"]) + "__" + property["json_suffix"]}.push_back(value.${property["json_conversion_method"]}());
            % endif
        }
        % elif property["json_type"] == "object":
        ${property["nested_type"]} ${"__".join(property["json_prefix"]) + "__" + property["json_suffix"]} {${property["type"]}Factory::create(${"__".join(property["json_prefix"])}.value(QLatin1String("${property["json_suffix"]}")).${property["json_conversion_method"]}(), parent)};
        % endif
        % endfor

        return ${nested_name}(
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
        );
    }
};
