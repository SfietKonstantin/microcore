% for nested_class in classes:
${nested_class["c_nested"]}
% endfor
${nested_name}::${name}
(
    % for i, property in enumerate(properties):
    % if i != len(properties) - 1:
    % if property["type_type"] == "simple":
    ${property["type"]} ${property["name"]},
    % else:
    ${property["type"]} &&${property["name"]},
    % endif
    % else:
    % if property["type_type"] == "simple":
    ${property["type"]} ${property["name"]}
    % else:
    ${property["type"]} &&${property["name"]}
    % endif
    % endif
    %endfor
)
    % for i, property in enumerate(properties):
    % if i == 0:
    % if property["type_type"] == "simple":
    : m_${property["name"]} {${property["name"]}}
    % else:
    : m_${property["name"]} {std::move(${property["name"]})}
    % endif
    % else:
    % if property["type_type"] == "simple":
    , m_${property["name"]} {${property["name"]}}
    % else:
    , m_${property["name"]} {std::move(${property["name"]})}
    % endif
    % endif
    % endfor
{
}

bool ${nested_name}::operator==(const ${name} &other) const
{
    % for i, property in enumerate(properties):
    if (m_${property["name"]} != other.m_${property["name"]}) {
        return false;
    }
    % endfor
    return true;
}

bool ${nested_name}::operator!=(const ${name} &other) const
{
    return !(*this == other);
}

% for property in properties:
% if property["type_type"] == "list":
std::vector<${property["nested_type"]}> ${nested_name}::${property["getter"]}() const
% else:
${property["nested_type"]} ${nested_name}::${property["getter"]}() const
% endif
{
    return m_${property["name"]};
}

% if property["access"] == "rw":
% if property["type_type"] == "simple":
void ${nested_name}::${property["setter"]}(${property["type"]} ${property["name"]})
% elif property["type_type"] == "list":
void ${nested_name}::${property["setter"]}(std::vector<${property["type"]}> &&${property["name"]})
% else:
void ${nested_name}::${property["setter"]}(${property["type"]} &&${property["name"]})
% endif
{
    % if property["type_type"] == "simple":
    m_${property["name"]} = ${property["name"]};
    % else:
    m_${property["name"]} = std::move(${property["name"]});
    % endif
}

% endif
% endfor