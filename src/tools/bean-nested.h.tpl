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
};