% for nested_class in classes:
${nested_class["c_nested"]}
% endfor
${nested_name}::${name}
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
${property["nested_type"]} ${nested_name}::${property["getter"]}() const
{
    return m_${property["name"]};
}

% if property["access"] == "rw":
void ${nested_name}::${property["setter"]}(${property["setter_type"]}${property["name"]})
{
    m_${property["name"]} = ${property["setter_impl"]};
}

% endif
% endfor