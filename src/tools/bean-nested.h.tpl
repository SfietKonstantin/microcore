class ${name}
{
public:
% for nested_class in classes:
${nested_class["h_nested"]}
% endfor
    explicit ${name}() = default;
    explicit ${name}
    (
        % for i, property in enumerate(properties):
        % if i != len(properties) - 1:
        % if property["type_type"] == "simple":
        ${property["type"]} ${property["name"]},
        % elif property["type_type"] == "list":
        std::vector<${property["type"]}> &&${property["name"]},
        % else:
        ${property["type"]} &&${property["name"]},
        % endif
        % else:
        % if property["type_type"] == "simple":
        ${property["type"]} ${property["name"]}
        % elif property["type_type"] == "list":
        std::vector<${property["type"]}> &&${property["name"]}
        % else:
        ${property["type"]} &&${property["name"]}
        % endif
        % endif
        %endfor
    );
    DEFAULT_COPY_DEFAULT_MOVE(${name});
    bool operator==(const ${name} &other) const;
    bool operator!=(const ${name} &other) const;
    % for property in properties:
    % if property["type_type"] == "list":
    std::vector<${property["type"]}> ${property["getter"]}() const;
    % else:
    ${property["type"]} ${property["getter"]}() const;
    % endif
    % if property["access"] == "rw":
    % if property["type_type"] == "simple":
    void ${property["setter"]}(${property["type"]} ${property["name"]});
    % elif property["type_type"] == "list":
    void ${property["setter"]}(std::vector<${property["type"]}> &&${property["name"]});
    % else:
    void ${property["setter"]}(${property["type"]} &&${property["name"]});
    % endif
    % endif
    % endfor
private:
    % for property in properties:
    % if property["type_type"] == "list":
    std::vector<${property["type"]}> m_${property["name"]} {};
    % else:
    ${property["type"]} m_${property["name"]} {${property["initial_value"]}};
    % endif
    % endfor
};
